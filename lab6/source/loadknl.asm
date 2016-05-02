;汇编程序源代码（bootOS.asm）
;%define	_BOOT_DEBUG_	;用于生成.COM文件易于调试

%ifdef	_BOOT_DEBUG_
	org  100h			; 调试状态，做成 .COM 文件, 可调试
%else
	org  7c00h			; BIOS将把引导扇区加载到0:7C00处，并开始执行
%endif

;==============================================================
%ifdef	_BOOT_DEBUG_
BaseOfStack		equ	100h	; 堆栈基地址(栈底, 从这个位置向低地址生长)
%else
BaseOfStack		equ	7c00h	; 堆栈基地址(栈底, 从这个位置向低地址生长)
%endif

BaseOfkernal	equ	0000h	; kernal.BIN 被加载到的位置 ----  段地址
OffsetOfkernal	equ	7e00h	; kernal.BIN 被加载到的位置 ---- 偏移地址
FATSegment		equ 8000h	; 存放FAT的临时位置 -- 段
RootDirSectors	equ	14		; 根目录占用的扇区数
SectorNoOfRootDirectory	equ	19	; 根目录区的首扇区号
SectorNoOfFAT1	equ	1		; FAT#1的首扇区号 = BPB_RsvdSecCnt
DeltaSectorNo	equ	17		; DeltaSectorNo = BPB_RsvdSecCnt + 
							; (BPB_NumFATs * FATSz) - 2 = 1 + (2*9) -2 = 17
							; 文件的开始扇区号 = 目录条目中的开始扇区号 
							; + 根目录占用扇区数目 + DeltaSectorNo
							; 用户数据空间的第一个簇编号为002
;==============================================================

	jmp short LABEL_START		; 引导开始，跳转指令
	nop							; 这个 nop 不可少，无操作，占字节位

	; 下面是 FAT12 磁盘引导扇区的BPB和EBPB结构区51字节
	BS_OEMName		DB 'MiraiOS '	; OEM串，必须8个字节，不足补空格
	BPB_BytsPerSec	DW 512		; 每扇区字节数
	BPB_SecPerClus	DB 1		; 每簇扇区数
	BPB_RsvdSecCnt	DW 1		; Boot记录占用扇区数
	BPB_NumFATs		DB 2		; FAT表数
	BPB_RootEntCnt	DW 224		; 根目录文件数最大值
	BPB_TotSec16	DW 2880		; 逻辑扇区总数
	BPB_Media		DB 0F0h		; 介质描述符
	BPB_FATSz16		DW 9		; 每FAT扇区数
	BPB_SecPerTrk	DW 18		; 每磁道扇区数
	BPB_NumHeads	DW 2		; 磁头数(面数)
	BPB_HiddSec		DD 	0			; 隐藏扇区数
	BPB_TotSec32	DD 0		; BPB_TotSec16为0时由此值记录扇区总数
	BS_DrvNum		DB 0		; 中断 13 的驱动器号（软盘）
	BS_Reserved1	DB 0		; 未使用
	BS_BootSig		DB 29h		; 扩展引导标记 (29h)
	BS_VolID		DD 0		; 卷序列号
	BS_VolLab		DB 'MiraiOS    '; 卷标，必须11个字节，不足补空格
	BS_FileSysType	DB 'FAT12   '	; 文件系统类型，必须8个字节，不足补空格

LABEL_START:
	mov	ax, cs	; 置其他段寄存器值与CS相同
	mov	ds, ax	; 数据段
	mov	es, ax	; 附加段
	mov	ss, ax	; 堆栈段
	mov	sp, BaseOfStack ; 堆栈基址

; 清屏
	mov ax, 3h
	int	10h			; 显示服务BIOS调用

	mov	dh, 0		; "Booting  "
	call	DispStr		; 显示字符串

; 软驱复位
	xor	ah, ah	; 功能号ah=0（复位磁盘驱动器）
	xor	dl, dl	; dl=0（软驱，硬盘和U盘为80h）
	int	13h		; 磁盘服务BIOS调用
	
; 下面在A盘根目录中寻找 kernal.BIN
	mov	word [wSectorNo], SectorNoOfRootDirectory 	; 给表示当前扇区号的
						; 变量wSectorNo赋初值为根目录区的首扇区号（=19）
LABEL_SEARCH_IN_ROOT_DIR_BEGIN:
	cmp	word [wRootDirSizeForLoop], 0	; 判断根目录区是否已读完
	jz	LABEL_NO_kernalBIN		;若读完则表示未找到kernal.BIN
	dec	word [wRootDirSizeForLoop]	; 递减变量wRootDirSizeForLoop的值
	; 调用读扇区函数读入一个根目录扇区到装载区
	mov	ax, BaseOfkernal
	mov	es, ax			; ES <- BaseOf kernal（9000h）
	mov	bx, OffsetOfkernal; BX <- OffsetOf kernal（100h）
	mov	ax, [wSectorNo]	; AX <- 根目录中的当前扇区号
	mov	cl, 1			; 只读一个扇区

	call	ReadSector	; 调用读扇区函数

	mov	si, kernalFileName	; DS:SI -> " kernal  BIN"
	mov	di, OffsetOfkernal 	; ES:DI -> BaseOfkernal :0100

	cld					; 清除DF标志位
						; 置比较字符串时的方向为左/上[索引增加]
	mov	dx, 10h			; 循环次数=16（每个扇区有16个文件条目：512/32=16）
LABEL_SEARCH_FOR_kernalBIN:
	cmp	dx, 0			; 循环次数控制
	jz LABEL_GOTO_NEXT_SECTOR_IN_ROOT_DIR ; 若已读完一扇区
	dec	dx				; 递减循环次数值			  就跳到下一扇区
	mov	cx, 11			; 初始循环次数为11

LABEL_CMP_FILENAME:
	cmp	cx, 0
	jz	LABEL_FILENAME_FOUND	; 如果比较了11个字符都相等，表示找到
	dec	cx				; 递减循环次数值
	lodsb				; DS:SI -> AL（装入字符串字节）
	cmp	al, byte [es:di]		; 比较字符串的当前字符
	jz	LABEL_GO_ON
	jmp	LABEL_DIFFERENT	; 只要发现不一样的字符就表明本DirectoryEntry
							; 不是我们要找的kernal.BIN
LABEL_GO_ON:
	inc	di					; 递增DI
	jmp	LABEL_CMP_FILENAME	; 继续循环

LABEL_DIFFERENT:
	and	di, 0FFE0h		; DI &= E0为了让它指向本条目开头（低5位清零）
					    ; FFE0h = 1111111111100000（低5位=32=目录条目大小）
	add	di, 20h			; DI += 20h 下一个目录条目
	mov	si, kernalFileName	; SI指向装载文件名串的起始地址
	jmp	LABEL_SEARCH_FOR_kernalBIN; 转到循环开始处

LABEL_GOTO_NEXT_SECTOR_IN_ROOT_DIR:
	add	word [wSectorNo], 1 ; 递增当前扇区号
	jmp	LABEL_SEARCH_IN_ROOT_DIR_BEGIN

LABEL_NO_kernalBIN:
	mov	dh, 2		; "No kernal "
	call DispStr	; 显示字符串
%ifdef	_BOOT_DEBUG_ ; 没有找到kernal.BIN就回到 DOS
	mov	ax, 4c00h	; AH=4Ch（功能号，终止进程）、AL=0（返回代码）
	int	21h			; DOS软中断
%else
	jmp	$			; 没有找到 kernal.BIN，在这里进入死循环
%endif

LABEL_FILENAME_FOUND:	; 找到 kernal.BIN 后便来到这里继续
	; 计算文件的起始扇区号
	mov	ax, RootDirSectors	; AX=根目录占用的扇区数
	and	di, 0FFE0h		; DI -> 当前条目的开始地址
	add	di, 1Ah			; DI -> 文件的首扇区号在条目中的偏移地址
	mov cx, word [es:di]	; CX=文件的首扇区号
	push cx				; 保存此扇区在FAT中的序号
	add	cx, ax			; CX=文件的相对起始扇区号+根目录占用的扇区数
	add	cx, DeltaSectorNo	; CL <- LOADER.BIN的起始扇区号(0-based)
	mov	ax, BaseOfkernal
	mov	es, ax			; ES <- BaseOf kernal（装载程序基址=9000h）
	mov	bx, OffsetOfkernal; BX <- OffsetOf kernal（装载程序偏移地址=100h）
	mov	ax, cx			; AX <- 起始扇区号

LABEL_GOON_LOADING_FILE:
	push bx				; 保存装载程序偏移地址
	mov	cl, 1				; 1个扇区
	call	ReadSector		; 读扇区

; 每读一个扇区就在 "Booting  " 后面打一个点, 形成这样的效果：Booting ......
	mov	ah, 0Eh		; 功能号（以电传方式显示单个字符）
	mov	al, '.'		; 要显示的字符
	mov	bl, 0Fh		; 黑底白字
	int	10h			; 显示服务BIOS调用

	; 计算文件的下一扇区号
	pop bx				; 取出装载程序偏移地址
	pop	ax				; 取出此扇区在FAT中的序号
	call	GetFATEntry	; 获取FAT项中的下一簇号
	cmp	ax, 0FF8h		; 是否是文件最后簇
	jae	LABEL_FILE_LOADED	; ≥FF8h时跳转，否则读下一个簇
	push ax				; 保存扇区在FAT中的序号
	mov	dx, RootDirSectors	; DX = 根目录扇区数 = 14
	add	ax, dx			; 扇区序号 + 根目录扇区数
	add	ax, DeltaSectorNo		; AX = 要读的数据扇区地址
	add	bx, [BPB_BytsPerSec]	; BX+512指向装载程序区的下一个扇区地址
	jmp	LABEL_GOON_LOADING_FILE
LABEL_FILE_LOADED:
	mov	dh, 1			; "Ready."
	call	DispStr		; 显示字符串

; **********************************************************************
	jmp	BaseOfkernal:OffsetOfkernal	; 这一句正式跳转到已加载到内
						; 存中的 kernal.BIN 的开始处，
						; 开始执行 kernal.BIN 的代码。
						; Boot Sector 的使命到此结束
; **********************************************************************

;==============================================================
;变量
wRootDirSizeForLoop	dw	RootDirSectors	; 根目录区剩余扇区数
										; 初始化为14，在循环中会递减至零
wSectorNo		dw	0	; 当前扇区号，初始化为0，在循环中会递增
bOdd			db	0	; 奇数还是偶数FAT项

;字符串
kernalFileName	db	"KERNEL  BIN", 0 ; kernal.BIN之文件名
; 为简化代码，下面每个字符串的长度均为MessageLength（=9），似串数组
MessageLength	equ	9
BootMessage:	db	"Booting  " 	; 9字节，不够则用空格补齐。序号0
Message1		db	"Ready.   " 	; 9字节，不够则用空格补齐。序号1
Message2		db	"No kernal"     ; 9字节，不够则用空格补齐。序号2
;==============================================================

;----------------------------------------------------------------------------
; 函数名：DispStr
;----------------------------------------------------------------------------
; 作用：显示一个字符串，函数开始时DH中须为串序号(0-based)
DispStr:
	mov	ax, MessageLength ; 串长->AX（即AL=9）
	mul	dh				; AL*DH（串序号）->AX（=当前串的相对地址）
	add	ax, BootMessage	; AX+串数组的起始地址
	mov	bp, ax			; BP=当前串的偏移地址
	mov	ax, ds			; ES:BP = 串地址
	mov	es, ax			; 置ES=DS
	mov	cx, MessageLength	; CX = 串长（=9）
	mov	ax, 1301h			; AH = 13h（功能号）、AL = 01h（光标置于串尾）
	mov	bx, 0007h		; 页号为0(BH = 0) 黑底白字(BL = 07h)
	mov	dl, 0			; 列号=0
	int	10h				; 显示服务BIOS调用
	ret					; 函数返回


;----------------------------------------------------------------------------

;----------------------------------------------------------------------------
; 函数名：ReadSector
;----------------------------------------------------------------------------
; 作用：从第 AX个扇区开始，将CL个扇区读入ES:BX中
ReadSector:
	; -----------------------------------------------------------------------
	; 怎样由扇区号求扇区在磁盘中的位置 (扇区号->柱面号、起始扇区、磁头号)
	; -----------------------------------------------------------------------
	; 设扇区号为 x
	;                             ┌ 柱面号 = y >> 1
	;       x              ┌ 商 y ┤
	;   -------------- 	=> ┤      └ 磁头号 = y & 1
	;  每磁道扇区数        │
	;                      └ 余 z => 起始扇区号 = z + 1
	push bp		; 保存BP
	mov bp, sp	; 让BP=SP
	sub	sp, 2 	; 辟出两个字节的堆栈区域保存要读的扇区数: byte [bp-2]
	mov	byte [bp-2], cl	; 压CL入栈（保存表示读入扇区数的传递参数）
	push bx			; 保存BX
	mov	bl, [BPB_SecPerTrk]	; BL=18（磁道扇区数）为除数
	div	bl			; AX/BL，商y在AL中、余数z在AH中
	inc	ah			; z ++（因磁盘的起始扇区号为1）
	mov	cl, ah		; CL <- 起始扇区号
	mov	dh, al		; DH <- y
	shr	al, 1		; y >> 1 （等价于y/BPB_NumHeads，软盘有2个磁头）
	mov	ch, al		; CH <- 柱面号
	and	dh, 1		; DH & 1 = 磁头号
	pop	bx			; 恢复BX
	; 至此，"柱面号、起始扇区、磁头号"已全部得到
	mov	dl, [BS_DrvNum]	; 驱动器号（0表示软盘A）
.GoOnReading: ; 使用磁盘中断读入扇区
	mov	ah, 2				; 功能号（读扇区）
	mov	al, byte [bp-2]		; 读AL个扇区
	int	13h				; 磁盘服务BIOS调用
	jc	.GoOnReading	; 如果读取错误，CF会被置为1，
						; 这时就不停地读，直到正确为止
	add	sp, 2			; 栈指针+2
	pop	bp				; 恢复BP

	ret
;----------------------------------------------------------------------------

;----------------------------------------------------------------------------
; 函数名：GetFATEntry
;----------------------------------------------------------------------------
; 作用：找到序号为AX的扇区在FAT中的条目，结果放在AX中。需要注意的
;     是，中间需要读FAT的扇区到ES:BX处，所以函数一开始保存了ES和BX
GetFATEntry:
	push es			; 保存ES、BX和AX（入栈）
	push bx
	push ax
; 设置读入的FAT扇区写入的基地址
	;mov ax,BaseOfkernal	;BaseOfKernal=9000h
	;sub	ax, 1000h	; 在BaseOfKernal后面留出4K空间用于存放FAT
	mov ax, FATSegment
	mov	es, ax		; ES=8000h
; 判断FAT项的奇偶
	pop	ax			; 取出FAT项序号（出栈）
	mov	byte [bOdd], 0; 初始化奇偶变量值为0（偶）
	mov	bx, 3		; AX*1.5 = (AX*3)/2
	mul	bx			; DX:AX = AX * 3（AX*BX 的结果值放入DX:AX中）
	mov	bx, 2		; BX = 2（除数）
	xor	dx, dx		; DX=0	
	div	bx			; DX:AX / 2 => AX <- 商、DX <- 余数
	cmp	dx, 0		; 余数 = 0（偶数）？
	jz LABEL_EVEN	; 偶数跳转
	mov	byte [bOdd], 1	; 奇数
LABEL_EVEN:		; 偶数
	; 现在AX中是FAT项在FAT中的偏移量，下面来
	; 计算FAT项在哪个扇区中(FAT占用不止一个扇区)
	xor	dx, dx		; DX=0	
	mov	bx, [BPB_BytsPerSec]	; BX=512
	div	bx			; DX:AX / 512
		  			; AX <- 商 (FAT项所在的扇区相对于FAT的扇区号)
		  			; DX <- 余数 (FAT项在扇区内的偏移)
	push dx			; 保存余数（入栈）
	mov bx, 0 		; BX <- 0 于是，ES:BX = 8000h:0
	add	ax, SectorNoOfFAT1 ; 此句之后的AX就是FAT项所在的扇区号
	mov	cl, 2			; 读取FAT项所在的扇区，一次读两个，避免在边界
	call	ReadSector	; 发生错误, 因为一个 FAT项可能跨越两个扇区
	pop	dx			; DX= FAT项在扇区内的偏移（出栈）
	add	bx, dx		; BX= FAT项在扇区内的偏移
	mov	ax, [es:bx]	; AX= FAT项值
	cmp	byte [bOdd], 1	; 是否为奇数项？
	jnz	LABEL_EVEN_2	; 偶数跳转
	shr	ax, 4			; 奇数：右移4位（取高12位）
LABEL_EVEN_2:		; 偶数
	and	ax, 0FFFh	; 取低12位
LABEL_GET_FAT_ENRY_OK:
	pop	bx			; 恢复ES、BX（出栈）
	pop	es
	ret
;----------------------------------------------------------------------------

	times 510-($-$$)	db	0	; 用0填充引导扇区剩下的空间
	db 	55h, 0aah				; 引导扇区结束标志
