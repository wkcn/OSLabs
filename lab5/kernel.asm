BITS 16
[global _start]
[extern main]

[global ShellMode]
[global RunNum]
[global PROG_SEGMENT]

[global INT09H_FLAG]
[global INT_INFO]

;16k = 0x4000
;4M = 0x4 0 0000
PCB_SEGMENT equ 3000h
PROG_SEGMENT_ equ 4000h
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
	mov ax,cs
	mov [es:si + 2],ax
%endmacro

;Init
	mov ax,cs
	mov ds,ax
	mov ax, 0
	mov ss, ax
	mov sp, 7c00h

	mov ax, cs
	mov es, ax
	mov ax, [es:09h * 4]
	mov word [INT09HORG], ax
	mov ax, [es:09h * 4 + 2]
	mov word [INT09HORG + 2], ax

	WriteIVT 08h,WKCNINTTimer ; Timer Interupt
	WriteIVT 09h,WKCNINT09H
	WriteIVT 20h,WKCNINT20H

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
	push es
	push ax
	mov ax, cs
	mov es, ax
	mov ax, %2
WAIT_INT_INFO_ZERO_%1: 
	sti
	cmp word [es:INT_INFO], 0
	jne WAIT_INT_INFO_ZERO_%1

	mov [es:INT_INFO], ax
	pop ax
	pop es
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
WKCNINT09H:	
	push es
	push ax

	mov ax, cs
	mov es, ax

	mov al,1
	xor byte [es:INT09H_FLAG], al

	sti
	pushf
	call far [es:INT09HORG]

	pop ax
	pop es
	iret

WKCNINT20H:

	push es
	push dx
	push bx
	push ax

	mov ax, cs
	mov es, ax
	mov ax, 0
	mov [es:ShellMode], ax
	mov ax, [es:RunID]
	mov bx, PCBSize
	mul bx
	mov bx, ax
	mov ax, PCB_SEGMENT
	mov es, ax
	mov al, 4
	mov byte[es:(bx + _STATE_OFFSET)], al
	pop ax
	pop bx
	pop dx
	pop es

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
	mov bx, word [ds:MaxRunNum]
	mov cx, PCBSize

	FindUserProg:
	inc ax
	cmp ax, bx
	jb MayExUserProg
	; 越界了
	mov ax, 0
	jmp GoodUserProg

	MayExUserProg:
	push ax
	mul cx
	mov si, ax
	pop ax
	cmp byte [es:(si + _STATE_OFFSET)], 1; Running
	je GoodUserProg

	cmp byte [es:(si + _STATE_OFFSET)], 3; Ready
	jne DEAD_JUDGE
	mov dl, 1
	mov byte [es:(si + _STATE_OFFSET)], dl
	;inc word [ds:RunNum]
	jmp GoodUserProg

	DEAD_JUDGE:
	cmp byte [es:(si + _STATE_OFFSET)], 4; Dead
	jne FindUserProg
	;Dead
	mov dl, 0
	mov byte [es:(si + _STATE_OFFSET)], dl
	dec word [ds:RunNum]
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
	SetOffset _NAME
	SetOffset _SIZE
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
	_STATE db 0 ; 结束态0, 运行态1
	_NAME db "0123456789ABCDEF" ; 16 bytes
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
