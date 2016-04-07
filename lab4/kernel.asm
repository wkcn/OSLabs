BITS 16
[global _start]
[extern main]
[global ShellMode]
[global GetKey]
[global RunID]
[global RunNum]
[global MaxRunNum]
[global PROG_SEGMENT]
[global KillProg]
[global WritePCB]

;16k = 0x4000
;4M = 0x4 0 0000
PCB_SEGMENT equ 3000h
PROG_SEGMENT_ equ 5000h
UserProgramOffset equ 100h
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
	mov ax, 0
	mov ss, ax
	mov sp, 7c00h

	
	WriteIVT 08h,WKCNINTTimer ; Timer Interupt
	WriteIVT 20h,WKCNINT20H
	;WriteIVT 21h,WKCNINT21H


_start:
	mov ax,cs
	mov ds,ax
	mov ss,ax
	;SetTimer
	mov al,34h
	out 43h,al ; write control word
	mov ax,1193182/UpdateTimes	;X times / seconds
	out 40h,al
	mov al,ah
	out 40h,al
	jmp main

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

%macro SaveReg 1
	mov ax, %1
	mov [es:(bx + _%1_OFFSET)], ax
%endmacro

%macro LoadReg 1
	mov ax, [es:(bx + _%1_OFFSET)]
	mov %1, ax
%endmacro

KillProg:
	cli
	;KillProg(dw runid)
	;runid must be not 0 !
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
	mov ax, 0
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

WKCNINT20H:
	;发送程序中止信号给内核
	push si
	push es
	push ax
	mov ax, 0x00
	mov es, ax
	mov ax, 0x7c00
	mov si, ax
	mov ax, 1
	mov [es:si], ax
	pop ax
	pop es
	pop si
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
	mov ax, [ds:ShellMode]
	cmp ax, 0
	je UseShell
	mov bx, word[ds:RunNum]
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

WritePCB:
	cli
	push bp
	push ds
	push es
	push dx
	push cx
	push bx
	push ax

	;开始计算PCB位置
	mov ax, cs
	mov ds, ax

	mov bx, PCB_SEGMENT
	mov es, bx

	mov ax, word[RunNum]
	mov bx, PCBSize
	mul bx
	;add ax, Processes
	mov bx, ax
	;bx = new progress PCB
	mov bp, sp
	mov cx, [ss:(bp + 2 + 2 + 2 * 7)]
	;cx = segment addr
	;es = kernel segment
	mov [es:(bx + _CS_OFFSET)], cx
	mov [es:(bx + _DS_OFFSET)], cx
	mov [es:(bx + _SS_OFFSET)], cx
	mov ax, UserProgramOffset
	mov [es:(bx + _IP_OFFSET)], ax
	sub ax, 4
	mov [es:(bx + _SP_OFFSET)], ax
	mov ax, 512
	mov [es:(bx + _FLAGS_OFFSET)], ax
	;分配进程ID
	mov ax, [ProcessIDAssigner]
	mov [es:(bx + _ID_OFFSET)], ax
	inc word[ProcessIDAssigner]
	inc word[RunNum]

	pop ax
	pop bx
	pop cx
	pop dx
	pop es
	pop ds
	pop bp

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
	MaxRunNum dw 16
	PROG_SEGMENT dw PROG_SEGMENT_
	ShellMode dw 0
	ProcessIDAssigner dw 1; 进程 ID 分配

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
