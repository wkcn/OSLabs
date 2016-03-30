BITS 16
[global _start]
[extern main]
[global RunProg]
[global KillProg]
[global ShellMode]
[global GetKey]
[global RunID]
[global RunNum]

PCB_SEGMENT equ 1000h
PROG_SEGMENT equ 0a00h
UserProgramOffset equ 100h
PROCESS_SIZE equ 1024
PROCESS_SEG_SIZE equ 1024 / 16 ; 以段计数
MaxRunNum equ 5 ; 最大运行进程数
UpdateTimes equ 20

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

	
	WriteIVT 08h,WKCNINTTimer ; Timer Interupt
	;WriteIVT 20h,WKCNINT20H
	;WriteIVT 21h,WKCNINT21H


_start:
	mov ax,cs
	mov ds,ax
	;SetTimer
	mov al,34h
	out 43h,al ; write control word
	mov ax,1193182/UpdateTimes	;X times / seconds
	out 40h,al
	mov al,ah
	out 40h,al
	jmp main

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

WKCNINT21H:
	iret

RunProg:
	cli
	;RunProg(dw sector)
	push bp
	push es
	push dx
	push cx
	push bx
	push ax

	mov ax, cs
	mov es, ax
	mov ax, [es:RunNum]
	mov bx, MaxRunNum
	cmp ax, bx
	;ax >= bx
	jnb OverRunNum


	mov bp, sp
	mov cx, [ss:(bp + 2 + 2 + 2 * 6)]

	;注意, 程序的段分配
	;主要是内存段不能重复,虽然代码段可重:-(
	mov ax, PROCESS_SEG_SIZE
	mov bx, [es:ProcessIDAssigner]
	mul bx
	add ax, PROG_SEGMENT 

	;ax = segment addr
	mov [es:AX_SAVE], ax; 保存段地址
	;设置段地址
	mov es, ax


	;用户程序地址偏移
	mov bx,UserProgramOffset
	mov ah,2				  ;功能号
	mov al,1				  ;扇区数
    mov dl,0                 ;驱动器号 ; 软盘为0，硬盘和U盘为80H
    mov dh,1                 ;磁头号 ; 起始编号为0
    mov ch,0                 ;柱面号 ; 起始编号为0
	sti
    int 13H					;调用读磁盘BIOS的13h功能
	cli

	;清屏
	;mov ax, 0003h
	;int 10h
	;开始计算PCB位置
	mov ax, cs
	mov es, ax
	mov ax, word[es:RunNum]
	mov bx, PCBSize
	mul bx
	add ax, Processes
	mov bx, ax
	;bx = new progress PCB
	mov ax, [es:AX_SAVE]
	;ax = segment addr


	;es = kernel segment
	mov [es:(bx + _CS_OFFSET)], ax
	mov [es:(bx + _DS_OFFSET)], ax
	mov [es:(bx + _SS_OFFSET)], ax
	mov ax, UserProgramOffset
	mov [es:(bx + _IP_OFFSET)], ax
	sub ax, 4
	mov [es:(bx + _SP_OFFSET)], ax
	mov ax, 512
	mov [es:(bx + _FLAGS_OFFSET)], ax
	;分配进程ID
	mov ax, [es:ProcessIDAssigner]
	mov [es:(bx + _ID_OFFSET)], ax
	inc word[es:ProcessIDAssigner]

	inc word[es:RunNum]

	;push word[es:AX_SAVE]
	;push 100h
	;retf
	OverRunNum:

	pop ax
	pop bx 
	pop cx
	pop dx
	pop es
	pop bp

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
	mov bx, word[ds:RunNum]
	cmp ax, bx
	jb VALIDID
	mov ax, 0
	VALIDID:
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
	mov bx, [ds:RunNum]
	cmp bx, 1; if eq, only shell but ShellMode = 1
	ja NotOnlyShell ; bx > 1
	;只有Shell, 强制切换回Shell
	mov ax, 0
	mov [ds:ShellMode],ax
	jmp UseShell
	NotOnlyShell:
	inc word[ds:RunID]
	mov ax, [ds:RunID]
	cmp ax, bx
	jb NOOVERRIDE ; < namely valid
	mov ax, 0
	mov word[ds:RunID], ax
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

	push ax
	mov al,20h
	out 20h,al
	out 0A0h,al
	pop ax

	sti
	iret

KillProg:
	cli
	;KillProg(dw sector)
	;Use si to cover di
	push es
	push bp
	push dx
	push cx
	push bx
	push ax
	mov bp, sp
	mov ax, [ss:(bp + 2 + 2 + 2 * 6)]
	mov bx, PCBSize
	mul bx
	mov di, ax; Set DI
	mov ax, cs; kernel
	mov es, ax
	dec word[es:RunNum]
	mov ax, [es:RunNum]
	mul bx 
	mov si, ax; Set SI
	mov bx, Processes
	mov cx, PCBSize
	COVERING:
		mov al, [es:bx + si]
		mov [es:bx + di], al
		inc bx
	loop COVERING
	mov bx, Processes
	mov ax, 0
	mov [es:bx + si + _CS_OFFSET],ax ; set orics = 0
	mov ax, cs; kernel
	mov es, ax
	mov [es:ShellMode], ax
	mov [es:RunID], ax
	pop ax
	pop bx
	pop cx
	pop dx
	pop bp
	pop es
	sti
	o32 ret

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
times PCBSize * MaxRunNum db 0
