BITS 16
[global _start]
[extern main]

[global ShellMode]
[global RunNum]

[global INT09H_FLAG]
[global INT_INFO]

;16k = 0x4000
;4M = 0x4 0 0000
MaxRunNum equ 32
MSG_SEGMENT equ 4000h
PCB_SEGMENT equ MSG_SEGMENT + 100h 
PROG_SEGMENT equ PCB_SEGMENT + (MaxRunNum * PCBSize / 16)
UserProgramOffset equ 100h
UpdateTimes equ 20

;写入中断向量表
%macro WriteIVT 2
	mov ax,%1
	mov bx,4
	mul bx
	mov si,ax
	mov ax,%2
	mov [cs:si],ax ; offset
	mov ax,cs
	mov [cs:si + 2],ax
%endmacro

;Init
	mov ax,cs
	mov ds,ax
	mov ax, 0
	mov ss, ax
	mov sp, 7c00h

	mov ax, [cs:09h * 4]
	mov word [INT09HORG], ax
	mov ax, [cs:09h * 4 + 2]
	mov word [INT09HORG + 2], ax

	WriteIVT 08h,WKCNINTTimer ; Timer Interupt
	WriteIVT 09h,WKCNINTKeyBoard
	WriteIVT 20h,WKCNINT20H ; 进程退出(为了简单, 这个中断只有一个功能)
	WriteIVT 21h,WKCNINT21H ; 进程功能
	WriteIVT 22h,WKCNINT22H ; 进程通信

	WriteIVT 33h,_INT_33h
	WriteIVT 34h,_INT_34h
	WriteIVT 35h,_INT_35h
	WriteIVT 36h,_INT_36h


_start:

	mov ax, PCB_SEGMENT
	mov es, ax
	mov al, 1
	mov byte [es:_STATE_OFFSET], al; 设置Shell为运行态
	
	;SetTimer
	mov al,34h
	out 43h,al ; write control word
	mov ax,1193182/UpdateTimes	;X times / seconds
	out 40h,al
	mov al,ah
	out 40h,al

	mov ax,cs
	mov ds,ax
	mov ss,ax
	mov es,ax

	jmp main


%macro IVT_INFO 2
; 中断号, 中断信号量
_INT_%1:
	push ax
	mov ax, %2
WAIT_INT_INFO_ZERO_%1: 
	sti
	cmp word [cs:INT_INFO], 0
	jne WAIT_INT_INFO_ZERO_%1

	mov [cs:INT_INFO], ax
	pop ax
	iret
%endmacro

IVT_INFO 33h,1 
IVT_INFO 34h,2 
IVT_INFO 35h,3 
IVT_INFO 36h,4 

%macro SaveReg 1
	mov ax, %1
	mov [es:(bx + _%1_OFFSET)], ax
%endmacro

%macro LoadReg 1
	mov ax, [es:(bx + _%1_OFFSET)]
	mov %1, ax
%endmacro

;键盘中断
WKCNINTKeyBoard:	
	push ax

	mov al,1
	xor byte [cs:INT09H_FLAG], al

	;sti
	pushf
	call far [cs:INT09HORG]

	pop ax
	iret

WKCNINT20H:

	push es
	push dx
	push bx
	push ax

	mov word[cs:ShellMode], 0
	mov ax, [cs:RunID]
	mov bx, PCBSize
	mul bx
	mov bx, ax
	mov ax, PCB_SEGMENT
	mov es, ax
	mov byte[es:(bx + _STATE_OFFSET)], 4 ; Dead

	pop ax
	pop bx
	pop dx
	pop es

	iret

;注意, 如果这个放在代码段, 会导致被当成代码运行:-(
INT21HJMPLIST:
	dw TabShellMode, TabProgState, GetRunID, RETN_PCB_S, RETN_PROG_S, RETN_MSG_S, RETN_MAXRUNNUM
	dw STOP_CLOCK, START_CLOCK, INC_RUNNUM

WKCNINT21H:
	;21H中断
	;AH = 00h, 切换ShellMode到al状态
	;AH = 01h, 切换进程状态到al
	;AH = 02h, 得到当前进程ID
	;AH = 03h, 返回PCB_SEGMENT
	;AH = 04h, 返回PROG_SEGMENT
	;AH = 05h, 返回MSG_SEGMENT
	;AH = 06h, 返回MaxRunNum
	;AH = 07h, 停止时钟
	;AH = 08h, 开启时钟
	;AH = 09h, ++RunNum
	push dx
	push cx
	push bx

	;判断是否有效
	cmp ah, 0x09
	ja INT21HEND

	xor bx, bx
	mov bl, ah
	shl bx, 1
	jmp word[cs:(INT21HJMPLIST + bx)]

	;AH = 00h
	TabShellMode:
	mov word[cs:ShellMode], ax; 已知ax高位为0
	jmp INT21HEND

	;AH = 01h
	TabProgState:
	mov dl, al
	mov ax, [cs:RunID] 
	mov bx, PCBSize
	mul bx
	mov bx, ax
	mov ax, PCB_SEGMENT
	push es
	mov es, ax
	mov byte[es:(bx + _STATE_OFFSET)], dl
	pop es
	jmp INT21HEND

	;AH = 02h
	GetRunID:
	mov ax, [cs:RunID]
	jmp INT21HEND

	RETN_PCB_S:
	mov ax, PCB_SEGMENT
	jmp INT21HEND

	RETN_PROG_S:
	mov ax, PROG_SEGMENT
	jmp INT21HEND

	RETN_MSG_S:
	mov ax, MSG_SEGMENT
	jmp INT21HEND

	RETN_MAXRUNNUM:
	mov ax, MaxRunNum
	jmp INT21HEND

	STOP_CLOCK:
	mov byte[cs:CLOCKON],0
	jmp INT21HEND

	START_CLOCK:
	mov byte[cs:CLOCKON],1
	jmp INT21HEND

	INC_RUNNUM:
	inc word[cs:RunNum] 
	jmp INT21HEND

	INT21HEND:

	pop bx
	pop cx
	pop dx
	iret

INT22HJMPLIST:
	dw MSG_READ, MSG_WRITE, SET_MSG_V,RESET_MSG,CLOSE_MSG,INT22HEND 

WKCNINT22H:
	;22H进程, 进程通信

	;ah = 00h 读
	;ah = 01h 写
	;ah = 02h 信号量设置(bh=0, 清零; bh=1, 加1; bh=2, 减1; bh=3, 设置为bl值)
	;ah = 03h 设置端口
	;ah = 04h 关闭端口
	;ah = 05h 只返回信号量
	;al = 端口值
	;基地址bx, 缓存大小cx, 段地址dx 
	;返回信号量(ax)

	;信号描述 大小
	;是否开启 byte
	;信号量 byte
	;大小	word
	;偏移量 word
	;段地址	word
	;
	;总大小 8
	
	MSG_OPENED_OFF equ 0
	MSG_SIG_OFF equ 1
	MSG_SIZE_OFF equ 2
	MSG_OFFSET_OFF equ 4
	MSG_SEG_OFF equ 6

	push dx
	push cx
	push bx
	push es
	push ds
	push si
	push di
	mov es, dx; es = 目标段地址
	mov di, bx
	;[es:di] = 目标量
	mov bl, ah; bl 记录功能号
	mov ah, 8
	mul ah
	;ax = 信号偏移量
	mov si, ax
	mov ax, MSG_SEGMENT
	mov ds, ax
	;[ds:si] = 信号量

	cmp bl, 0x05
	ja INT22HEND
	mov bh, 0
	shl bx, 1
	jmp word[cs:(INT22HJMPLIST + bx)]

%macro INIT_MSG_RW 1
	cmp byte[ds:si + MSG_OPENED_OFF], 0
	je INT22HEND
	cmp cx, word[ds:si + MSG_SIZE_OFF]
	jbe NOT_OVERFLOW_%1
	mov cx, word[ds:si + MSG_SIZE_OFF]
	NOT_OVERFLOW_%1:
	mov ax, [ds:si + MSG_SEG_OFF]
	mov bx, [ds:si + MSG_OFFSET_OFF]	
	cld
%endmacro

MSG_READ:
	INIT_MSG_RW 0
	mov ds, ax
	mov si, bx
	MSG_READ_LOOP:
		movsb
	loop MSG_READ_LOOP	
	jmp INT22HEND
MSG_WRITE:
	INIT_MSG_RW 1
	mov dx, es
	mov es, ax
	mov ds, dx
	mov si, di
	mov di, bx
	MSG_WRITE_LOOP:
		movsb
	loop MSG_WRITE_LOOP
	mov dx, di
	mov si, dx
	mov dx, es
	mov ds, dx
	jmp INT22HEND
SET_MSG_V:
	cmp byte[ds:si + MSG_OPENED_OFF], 0
	je INT22HEND
	;信号量设置(bh=0, 清零; bh=1, 加1; bh=2, 减1; bh=3, 设置为bl值)
	mov bx, di
	cmp bh, 0
	je SET_MSG_V_CLEAR
	cmp bh, 1
	je SET_MSG_V_ADD
	cmp bh, 2
	je SET_MSG_V_DEC
	cmp bh, 3
	je SET_MSG_V_NUM

	SET_MSG_V_CLEAR:
		mov byte[ds:si+MSG_SIG_OFF],0
		jmp INT22HEND
	SET_MSG_V_ADD:
		inc byte[ds:si+MSG_SIG_OFF]
		jmp INT22HEND
	SET_MSG_V_DEC:
		dec byte[ds:si+MSG_SIG_OFF]
		jmp INT22HEND
	SET_MSG_V_NUM:
		mov byte[ds:si+MSG_SIG_OFF],bl
		jmp INT22HEND

RESET_MSG:
	mov ax, 0x0001
	mov word[ds:si], ax	
	mov word[ds:si + MSG_SIZE_OFF], cx
	mov word[ds:si + MSG_OFFSET_OFF], di
	mov ax, es
	mov word[ds:si + MSG_SEG_OFF], ax
	jmp INT22HEND
CLOSE_MSG:
	xor al, al
	mov byte[ds:si], al
	jmp INT22HEND

	INT22HEND:
	;设置信号量
	mov al, byte[ds:si + MSG_SIG_OFF]
	pop di
	pop si
	pop ds
	pop es
	pop bx
	pop cx
	pop dx

	iret

WKCNINTTimer:
	cli
	;Save current Progress
	;System Stack: *\flags\cs\ip
	push ds
	;System Stack: *\flags\cs\ip\ds(old)
	push cs
	;System Stack: *\flags\cs\ip\ds(old)\cs(kernel)
	pop ds
	;ds = data segment(kernel)
	;System Stack: *\flags\cs\ip\ds(old)
	mov [ds:AX_SAVE], ax
	mov [ds:BX_SAVE], bx
	mov [ds:CX_SAVE], cx
	mov [ds:DX_SAVE], dx

	mov ax, word[ds:RunID]

	;Must have a progress, it is Shell :-)
	;ES,DS,DI,SI,BP,SP,BX,DX,CX,AX,SS,IP,CS,FLAGS
	mov bx,PCBSize
	mul bx
	;add ax, Processes; current process PCB
	mov bx,ax
	push ds
	mov ax, PCB_SEGMENT
	mov ds, ax
	mov ax, es
	mov [bx + _ES_OFFSET], ax 
	pop ds
	mov ax, PCB_SEGMENT
	mov es, ax
	;SaveReg ES
	SaveReg DI
	SaveReg SI
	SaveReg BP
	pop word[es:(bx + _DS_OFFSET)] ;System Stack: *\flags\cs\ip\
	nop; 如果不加这句,会丢失下面一条pop语句,奇怪的bug!
	pop word[es:(bx + _IP_OFFSET)]
	pop word[es:(bx + _CS_OFFSET)]
	pop word[es:(bx + _FLAGS_OFFSET)]
	;System Stack: *
	SaveReg SP
	mov ax, [ds:DX_SAVE]
	mov [es:(bx + _DX_OFFSET)], ax
	mov ax, [ds:CX_SAVE]
	mov [es:(bx + _CX_OFFSET)], ax
	mov ax, [ds:BX_SAVE]
	mov [es:(bx + _BX_OFFSET)], ax
	mov ax, ss
	mov [es:(bx + _SS_OFFSET)], ax
	mov ax, [ds:AX_SAVE]
	mov [es:(bx + _AX_OFFSET)], ax
	;All Saved!
	;Run Next Program!

	;进程调度
	;ax 是将要运行的进程id
	;可用寄存器, ax,bx
	;运行用户程序
	mov ax, word [ds:RunID]
	mov cx, PCBSize

	cmp byte [ds:CLOCKON], 1
	jne GoodUserProg ; 不等于1, 说明切换关闭
	;假如不是Running态, 不能继续运行
	cmp byte [es:(bx + _STATE_OFFSET)], 1
	jne FindUserProg
	mov dl, byte [ds:PRIORITY_COUNT]
	inc byte [ds:PRIORITY_COUNT]
	;bx 还是之前程序的PCB偏移
	cmp dl, byte [es:(bx + _PRIORITY_OFFSET)]
	; <= 不高于,不进行进程切换
	jna GoodUserProg
	mov byte [ds:PRIORITY_COUNT], 0

	FindUserProg:
	inc ax
	cmp ax, MaxRunNum
	jb MayExUserProg
	; 越界了
	mov ax, 0
	jmp GoodUserProg

	;可能是要执行的用户程序
	MayExUserProg:
	push ax
	mul cx
	mov si, ax
	pop ax

	xor dx, dx
	mov dl, byte [es:(si + _STATE_OFFSET)]
	shl dx, 1
	mov di, dx
	jmp word [cs:(ProgState + di)]

ProgState:
	;EMPTY, RUNNING, SUSPEND, READY, DEAD, BLOCKED
	dw FindUserProg, GoodUserProg, FindUserProg, ReadyState, DeadState, FindUserProg 

ReadyState:
	mov byte [es:(si + _STATE_OFFSET)], 1; Ready -> Running
	jmp GoodUserProg

DeadState:
	;Dead
	cmp byte [es:(si + _KIND_OFFSET)], 2; 若等于则为线程 
	jne KillCommonProg
	;线程终结处理
	jmp FindUserProg ; 暂时处理方法为: 由主线程统一处理

%macro ReleaseProg 0
	mov byte[cs:CLOCKON],0
	push cx
	push bx
	push ax

	;释放进程所占内存
	mov ax, 0
	mov bx, word [es:(si + _SEG_OFFSET)]
	mov cx, word [es:(si + _SSIZE_OFFSET)]
	sti
	int 23h

	;清除信号量
	mov ah, 05h
	mov al, byte [es:(si + _ID_OFFSET)]
	sti
	int 25h

	mov byte [es:(si + _STATE_OFFSET)], 0
	dec word [ds:RunNum]

	pop ax
	pop bx
	pop cx

	mov byte[cs:CLOCKON],1
%endmacro

	KillCommonProg:
	ReleaseProg

	;杀死线程
	mov bx, ax
	push ax
	mov ax, MaxRunNum - 1
	KillThread:
		push ax
		mul cx
		mov si, ax
		pop ax
		cmp byte [es:(si + _STATE_OFFSET)], 0 ; 空进程
		je NOTTHREAD
		cmp byte [es:(si + _KIND_OFFSET)], 2; 等于则线程
		jne NOTTHREAD
		cmp byte [es:(si + _PARENT_ID_OFFSET)], bl
		jne NOTTHREAD
		ReleaseProg
		NOTTHREAD:
		dec ax
	jg KillThread 
	pop ax


	jmp FindUserProg

	GoodUserProg:
	mov word[ds:RunID], ax

LOAD_PCB:
	;Parameter: ax = RunID
	;Stack: *
	;Restart RunID(ax)
	;Must have a progress, it is Shell :-)
	;ES,DS,DI,SI,BP,SP,BX,DX,CX,AX,SS,IP,CS,FLAGS
	mov bx,PCBSize
	mul bx
	;add ax, Processes; current process PCB
	mov bx, ax
	;Now DS is kernel DS
	LoadReg SP
	mov ax, word[es:(bx + _SS_OFFSET)]
	mov ss, ax
	mov ax, word[es:(bx + _FLAGS_OFFSET)]
	push ax
	mov ax, word[es:(bx + _CS_OFFSET)]
	push ax
	mov ax, word[es:(bx + _IP_OFFSET)]
	push ax
	LoadReg DI
	LoadReg SI
	LoadReg BP
	;LoadReg ES

	mov ax, es
	mov ds, ax
	mov ax, [es:(bx + _ES_OFFSET)]
	mov es, ax

	mov cx, [bx + _CX_OFFSET]
	mov dx, [bx + _DX_OFFSET]
	mov ax, [bx + _DS_OFFSET]
	push ax
	mov ax, [bx + _BX_OFFSET]
	push ax
	;*/flags/cs/ip/ds/bx
	mov ax, [bx + _AX_OFFSET]
	pop bx
	pop ds

	push ax
	mov al,20h
	out 20h,al
	out 0A0h,al
	pop ax
	sti

	iret

%macro SetOffset 1
	%1_OFFSET equ (%1 - Processes)
%endmacro

DATA:
	AX_SAVE dw 0
	BX_SAVE dw 0
	CX_SAVE dw 0
	DX_SAVE dw 0
IVT:
	INT_INFO dw 0
	INT09HORG dd 0
	INT09H_FLAG db 0
PCBCONST:
	PCBSize equ FirstProcessEnd - Processes
	SetOffset _ID
	SetOffset _STATE
	SetOffset _NAME
	SetOffset _SIZE
	SetOffset _SSIZE
	SetOffset _SEG
	SetOffset _KIND
	SetOffset _PARENT_ID
	SetOffset _PRIORITY
	SetOffset _ES
	SetOffset _DS
	SetOffset _DI
	SetOffset _SI
	SetOffset _BP
	SetOffset _SP
	SetOffset _BX
	SetOffset _DX
	SetOffset _CX
	SetOffset _AX
	SetOffset _SS
	SetOffset _IP
	SetOffset _CS
	SetOffset _FLAGS
ProcessesTable:
	RunID dw 0 ; default to open shell
	RunNum dw 1
	ShellMode dw 0
	PRIORITY_COUNT db 0
	ProcessIDAssigner dw 1; 进程 ID 分配
	CLOCKON db 1

Processes:
	_ID db 0
	_STATE db 0 ; 结束态0, 运行态1
	_NAME db "0123456789ABCDEF" ; 16 bytes
	_KIND db 0
	_PARENT_ID db 0
	_BLOCK_NEXT db 0
	_PRIORITY db 0
	_SEG dw 0
	_SSIZE dw 0
	_SIZE dw 0
	_ES dw 0
	_DS dw 0
	_DI dw 0
	_SI dw 0
	_BP dw 0
	_SP dw 0
	_BX dw 0
	_DX dw 0
	_CX dw 0
	_AX dw 0
	_SS dw 0
	_IP dw 0
	_CS dw 0
	_FLAGS dw 512
FirstProcessEnd:
