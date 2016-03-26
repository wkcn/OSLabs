BITS 16
[global _start]
[extern main]
[global RunProg]
[global OK]

UserProgramOffset equ 100h

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
	WriteIVT 20h,WKCNINT20H
	WriteIVT 21h,WKCNINT21H


_start:
	mov ax,cs
	mov ds,ax
	mov ss,ax
	call SetTimer
	jmp main

OK:
	;WriteIVT 08h,WKCNINTTimer ; Timer Interupt
	ret

SetTimer:
	mov al,34h
	out 43h,al ; write control word
	mov ax,1193182/20	;X times / seconds
	out 40h,al
	mov al,ah
	out 40h,al
	ret

CLEARSCREEN:
	mov ax, 0003h
	int 10h
	iret

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
	call 0:CLEARSCREEN
	jmp 0:_start
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
	mov bp, sp
	mov cx, [ss:(bp + 2 + 2)]

	;设置段地址
	mov ax,0A00H
	mov es,ax
	mov ss,ax

	;用户程序地址偏移
	mov bx,UserProgramOffset
	mov ah,2				  ;功能号
	mov al,1				  ;扇区数
    mov dl,0                 ;驱动器号 ; 软盘为0，硬盘和U盘为80H
    mov dh,0                 ;磁头号 ; 起始编号为0
    mov ch,0                 ;柱面号 ; 起始编号为0
    int 13H ;	              调用读磁盘BIOS的13h功能

	;清屏
	mov ax, 0003h
	int 10h
	;执行用户程序

	pop ax; short ip
	mov ax,0A00H
	mov es,ax
	push es
	push UserProgramOffset
	retf

%macro SaveReg 1
	mov ax, %1
	mov [bx + _%1_OFFSET], ax
%endmacro

%macro LoadReg 1
	mov ax, [bx + _%1_OFFSET]
	mov %1, ax
%endmacro

WKCNINTTimer:
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
	add ax, Processes; current process PCB
	mov bx,ax
	SaveReg ES
	SaveReg DI
	SaveReg SI
	SaveReg BP
	pop word[bx + _DS_OFFSET]
	;System Stack: *\flags\cs\ip\
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
	inc word[RunID]
	mov ax, [RunID]
	mov bx, [RunNum]
	cmp ax, bx
	jb NOOVERRIDE
	mov ax, 0
	mov [RunID], ax
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
	mov ax, [bx + _SS_OFFSET]
	mov ss, ax
	mov ax, [bx + _FLAGS_OFFSET]
	push ax
	mov ax, [bx + _CS_OFFSET]
	push ax
	mov ax, [bx + _IP_OFFSET]
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

	iret

	 

	;Get PCB

%macro SetOffset 1
	%1_OFFSET equ %1 - Processes
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
	_FLAGS dw 0
FirstProcessEnd:
