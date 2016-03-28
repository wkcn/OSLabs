BITS 16
[global _start]
[extern main]
[global RunProg]
[global KillProg]
[global KillAll]
[global ShellMode]
[global GetKey]
[global RunNum]

PCB_SEGMENT equ 2000h
PROG_SEGMENT equ 3000h
UserProgramOffset equ 100h
PROCESS_SIZE equ 1024 / 16 ; 以段计数
cli

;写入中断向量表
%macro WriteIVT 2
	mov ax,0000h
	mov es,ax

	mov ax,%1
	mov bx,4
	mul bx
	mov si,ax
	mov ax,%2
	mov [es:si],ax ; offset
	add si,2
	mov ax,cs
	mov [es:si],ax
%endmacro

;Init
	mov ax,cs
	mov ds,ax
	;call InitProgs

	WriteIVT 08h,WKCNINTTimer ; Timer Interupt
	;WriteIVT 20h,WKCNINT20H
	;WriteIVT 21h,WKCNINT21H


_start:
	mov ax,cs
	mov ds,ax
	call SetTimer
	sti
	jmp main

;InitProgs:
;	mov cx, 16
;	IPLOOP:
;		mov ax, Processes
;		add ax, PCBSize
;		mov es, ax
;		mov ax, 0A00H
;		;mov [es:
;	loop LPLOOP
;	ret

SetTimer:
	mov al,34h
	out 43h,al ; write control word
	mov ax,1193182/200	;X times / seconds
	out 40h,al
	mov al,ah
	out 40h,al
	ret

CLEARSCREEN:
	mov ax, 0003h
	int 10h
	iret

GetKey:
	
	mov ah,01h
	int 16h
	jz  NOKEY	;没有按键
	;按键了,获取字符
	mov ah,00h
	int 16h
	jmp HAVEKEY
	mov ax, 0
	NOKEY:
	HAVEKEY:

	o32 ret


WKCNINT20H:
	;input ctrl+z, and quit
	push ax
	mov ah,01h
	int 16h
	jz  NOCZ	;没有按键
	;按键了,获取字符
	mov ah,00h
	int 16h
	cmp ax,2c1ah
	jne NOCZ    ; 如果没有按Ctrl + Z, 跳转到NOCZ

	pop ax	;to int 21h

	int 21h

	NOCZ:
	pop ax	;to iret
	iret

WKCNINT21H:
	;返回Shell
	;call 0:CLEARSCREEN
	;jmp 0:_start
	mov ax, 0
	mov es, ax
	mov ax, [es:RunID]
	push 0
	push _start
	push ax
	jmp 0:KillProg
	iret

WKCNINTTimer2:
	push ax
	mov al, 20h
	out 20h, al ;send EOI to +8529A
	out 0A0h,al	;send EOI to -8529A
	pop ax
	int 20h
	iret

RunProg:
	;RunProg(dw sector)
	cli ; 屏蔽中断
	push bp
	push es
	push dx
	push cx
	push bx
	push ax
	mov bp, sp
	mov cx, [ss:(bp + 2 + 2 + 2 * 6)]

	mov ax, PROCESS_SIZE
	mul cx
	add ax, PROG_SEGMENT 


	CREATE_PROCESS:

	;ax = segment addr
	mov [es:AX_SAVE], ax; 保存段地址
	;设置段地址
	mov es, ax


	;用户程序地址偏移
	mov bx,UserProgramOffset
	mov ah,2				  ;功能号
	mov al,1				  ;扇区数
    mov dl,0                 ;驱动器号 ; 软盘为0，硬盘和U盘为80H
    mov dh,0                 ;磁头号 ; 起始编号为0
    mov ch,0                 ;柱面号 ; 起始编号为0
    int 13H ;	              调用读磁盘BIOS的13h功能

	;清屏
	;mov ax, 0003h
	;int 10h
	;开始计算PCB位置
	mov ax, 0
	mov es, ax
	mov ax, word[es:RunNum]
	mov bx, PCBSize
	mul bx
	add ax, Processes
	mov bx, ax
	;bx = new progress PCB
	mov ax, [es:AX_SAVE]
	;ax = segment addr


	;ds = 0
	mov [bx + _CS_OFFSET], ax
	mov [bx + _DS_OFFSET], ax
	mov [bx + _SS_OFFSET], ax
	mov ax, UserProgramOffset
	mov [bx + _IP_OFFSET], ax
	sub ax, 4
	mov [bx + _SP_OFFSET], ax
	mov ax, 512
	mov [bx + _FLAGS_OFFSET], ax
	;分配进程ID
	mov ax, [es:ProcessIDAssigner]
	mov [bx + _ID_OFFSET], ax
	inc word[es:ProcessIDAssigner]

	
	mov ax, 0
	mov es, ax
	inc word[es:RunNum]

	pop ax
	pop bx 
	pop cx
	pop dx
	pop es
	pop bp


	push ax
	mov al, 20h
	out 20h, al ;send EOI to +8529A
	out 0A0h,al	;send EOI to -8529A
	pop ax

	sti ; 恢复中断
	
	o32 ret

KillAll:
	;杀死所有进程, 除了Shell
	cli
	push es
	push ax
	mov ax, 0
	mov es, ax
	mov [es:RunID], ax
	mov ax, 1
	mov [es:RunNum], ax
	pop ax
	pop es
	o32 ret

KillProg:
	;杀死进程KillProg(dw killid)
	cli
	push si
	push di
	push bp
	push es
	push dx
	push cx
	push bx
	push ax

	xor ax, ax; ax = 0
	mov es, ax
	mov bp, sp
	mov ax, [ss:(bp + 2 + 2 + 2 * 8)]
	cmp ax, 0
	je UNVALID; 企图杀死0号进程,无效

	;SI,DI
	;mov bx, PCBSize
	;mul bx
	;mov si, ax
	mov cx, Processes
	add cx, PCBSize; 跳过0号进程
	mov si, cx
	mov cx, [RunNum]
	; es = 0
	FIND_PROCESS:
		mov bx, [es:(si + _ID_OFFSET)]
		cmp ax, bx
		je FINDED_PROCESS
		add si, PCBSize
	loop FIND_PROCESS

	jmp UNVALID; 没有找到进程
	FINDED_PROCESS:
	sub si, Processes
	mov ax, [RunNum]
	dec ax
	mul bx
	mov di, ax
	;用D覆盖S

	mov bx, Processes
	mov cx, PCBSize

	mov ax, [es:(bx + si + _CS_OFFSET)]
	cmp ax, 0
	je UNVALID; 无效进程号

	COVER_MEM:
		;byte
		mov al, [es:(bx + di)]
		mov [es:(bx + si)], al
	loop COVER_MEM
	mov ax, [RunNum]
	dec ax
	mov cx, PCBSize
	mul cx
	mov si, ax
	mov ax, 0
	;设置PCB_CS为0
	mov [es:(bx + si + _CS_OFFSET)], ax
	dec word[RunNum]

	UNVALID:
	pop ax
	pop bx
	pop cx
	pop dx
	pop es
	pop bp
	pop di
	pop si
	sti
	o32 ret


%macro SaveReg 1
	mov ax, %1
	mov [bx + _%1_OFFSET], ax
%endmacro

%macro LoadReg 1
	mov ax, [bx + _%1_OFFSET]
	mov %1, ax
%endmacro


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

	mov ax, word[ds:ShellMode]
	cmp ax, 0
	je ShellRunning
	mov ax, word[ds:RunID]
	ShellRunning:

	;Must have a progress, it is Shell :-)
	;ES,DS,DI,SI,BP,SP,BX,DX,CX,AX,SS,IP,CS,FLAGS
	mov bx,PCBSize
	mul bx
	add ax, Processes; current process PCB
	mov bx,ax
	SaveReg ES
	SaveReg DI
	SaveReg SI
	SaveReg BP
	pop word[bx + _DS_OFFSET] ;System Stack: *\flags\cs\ip\
	nop; 如果不加这句,会丢失下面一条pop语句,奇怪的bug!
	pop word[bx + _IP_OFFSET]
	pop word[bx + _CS_OFFSET]
	pop word[bx + _FLAGS_OFFSET]
	;System Stack: *
	SaveReg SP
	mov ax, [ds:DX_SAVE]
	mov [bx + _DX_OFFSET], ax
	mov ax, [ds:CX_SAVE]
	mov [bx + _CX_OFFSET], ax
	mov ax, [ds:BX_SAVE]
	mov [bx + _BX_OFFSET], ax
	mov ax, ss
	mov [bx + _SS_OFFSET], ax
	mov ax, [ds:AX_SAVE]
	mov [bx + _AX_OFFSET], ax
	;All Saved!
	;Run Next Program!

	;进程调度
	;ax 是将要运行的进程id
	;可用寄存器, ax,bx
	mov ax, [ds:ShellMode]
	cmp ax, 0
	je UseShell
	inc word[ds:RunID]
	mov bx, [ds:RunNum]
	cmp bx, 1; if eq, only shell but ShellMode = 1
	jne NotOnlyShell
	;只有Shell, 强制切换回Shell
	mov ax, 0
	mov [ds:ShellMode],ax
	jmp UseShell
	NotOnlyShell:
	mov ax, [ds:RunID]
	cmp ax, bx
	jb NOOVERRIDE ; < namely valid
	mov ax, 0
	mov [ds:RunID], ax
	UseShell:

	NOOVERRIDE:
	;Restart RunID(ax)
	;Must have a progress, it is Shell :-)
	;ES,DS,DI,SI,BP,SP,BX,DX,CX,AX,SS,IP,CS,FLAGS
	mov bx,PCBSize
	mul bx
	add ax, Processes; current process PCB
	mov bx, ax
	;Now DS is kernel DS
	LoadReg SP
	mov ax, word[bx + _SS_OFFSET]
	mov ss, ax
	mov ax, word[bx + _FLAGS_OFFSET]
	push ax
	mov ax, word[bx + _CS_OFFSET]
	push ax
	mov ax, word[bx + _IP_OFFSET]
	push ax
	LoadReg ES
	LoadReg DI
	LoadReg SI
	LoadReg BP
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

	mov al,20h
	out 20h,al
	out 0A0h,al
	sti
	iret

	 

	;Get PCB

%macro SetOffset 1
	%1_OFFSET equ (%1 - Processes)
%endmacro

DATA:
	AX_SAVE dw 0
	BX_SAVE dw 0
	CX_SAVE dw 0
	DX_SAVE dw 0
PCBCONST:
	PCBSize equ FirstProcessEnd - Processes
	SetOffset _ID
	SetOffset _NAME
	SetOffset _STATE
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
	ProcessIDAssigner dw 1;进程ID分配器
	ShellMode dw 0
Processes:
	_ID db 0
	_STATE db 0
	_NAME db "0123456789ABCDEF" ; 16 bytes
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

%macro PCB 1
	_ID%1 db 0
	_STATE%1 db 0
	_NAME%1 db "0123456789ABCDEF" ; 16 bytes
	_ES%1 dw 0
	_DS%1 dw 0
	_DI%1 dw 0
	_SI%1 dw 0
	_BP%1 dw 0
	_SP%1 dw 0
	_BX%1 dw 0
	_DX%1 dw 0
	_CX%1 dw 0
	_AX%1 dw 0
	_SS%1 dw 0
	_IP%1 dw 0
	_CS%1 dw 0
	_FLAGS%1 dw 512
%endmacro
times PCBSize * 16 db 0
