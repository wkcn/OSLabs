BITS 16
[global _start]
[extern main]
[global RunProg]


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
	mov ax, cs
	mov ds, ax
	;call SetTimer
	jmp main

SetTimer:
	mov al,34h
	out 43h,al ; write control word
	mov ax,1193182/20	;X times / seconds
	out 40h,al
	mov al,ah
	out 40h,al
	ret

CLEARSCREEN:
	mov ax, 03h
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

WKCNINTTimer:
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
	mov ax, ss
	mov es, ax
	mov cx, [es:(bp + 2 + 2)]

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

	call 0:CLEARSCREEN
	;执行用户程序
	pop ax; short ip
	push es
	push UserProgramOffset
	retf

Data:
	sectorID dw 0

ProcessesTable:
	processNum dw 1 ; default to open shell
Processes:
