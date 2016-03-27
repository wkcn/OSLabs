BITS 16
[global _start]
[extern main]
[global RunProg]
[global OK]

UserProgramOffset equ 100h
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

	WriteIVT 08h,WKCNINTTimer ; Timer Interupt
	WriteIVT 20h,WKCNINT20H
	WriteIVT 21h,WKCNINT21H


_start:
	mov ax,cs
	mov ds,ax
	call SetTimer
	sti
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
	cli ; 屏蔽中断
	push bp
	push es
	push dx
	push cx
	push bx
	push ax
	mov bp, sp
	mov cx, [ss:(bp + 2 + 2 + 2 * 6)]

	mov cx, 11

	mov ax, 0	;切换到内核段
	mov es, ax
	mov ax, [es:RunNum]
	mov bx, 1024 / 16
	mul bx
	add ax, 0A00H
	

	mov ax, 0A00H
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
	mov ax, [es:RunNum]
	mov bx, PCBSize
	mul bx
	add ax, Processes
	mov bx, ax
	;bx = new progress PCB
	mov ax, [es:AX_SAVE]
	mov es, bx
	;ax = segment addr
	mov ax, 0A00H
	mov [es:_CS_OFFSET], ax
	mov [es:_DS_OFFSET], ax
	mov [es:_SS_OFFSET], ax
	mov ax, UserProgramOffset
	mov [es:_IP_OFFSET], ax
	sub ax, 4
	mov [es:_SP_OFFSET], ax
	mov ax, 512
	mov [es:_FLAGS_OFFSET], ax

	
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

;单进程处理
RunProg2:
	;RunProg(dw sector)
	mov bp, sp
	mov cx, [ss:(bp + 2 + 2)]

	;设置段地址
	mov ax,0A00H
	mov es,ax

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
	inc word[ds:RunID]
	mov ax, [ds:RunID]
	mov bx, [ds:RunNum]
	cmp ax, bx
	jb NOOVERRIDE ; < namely valid
	mov ax, 0
	mov [ds:RunID], ax
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
	_ID2 db 0
	_STATE2 db 0
	_NAME2 db "0123456789ABCDEF" ; 16 bytes
	_ES2 dw 0
	_DS2 dw 0A00H
	_DI2 dw 0
	_SI2 dw 0
	_BP2 dw 0
	_SP2 dw 100H-4
	_BX2 dw 0
	_DX2 dw 0
	_CX2 dw 0
	_AX2 dw 0
	_SS2 dw 0A00H
	_IP2 dw 100H
	_CS2 dw 0A00H
	_FLAGS2 dw 512

