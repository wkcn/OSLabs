BITS 16
[global _start]
[extern main]

_start:
	mov ax, cs
	mov ds, ax
	mov ax, 0
	mov ss, ax
	mov sp, 7c00h

	;清屏
	mov ax, 3
	int 10h

	;视频模式
	mov ah, 0
	mov al, 12h
	int 10h

	mov bx, 0;PIC
	mov cx, 0
	mov dx, 0
	call DRAW

	jmp $

;Draw
;bx = offset PIC
;cx = column
;dx = row
;32 * 32
DRAW:
	push ax
	mov [cs:DrawCol], cx
	mov word [cs:DrawCount], 0
	mov word [cs:DrawXCount], 0
	DRAWPOT:

	mov al, byte[cs:bx + PIC]
	push bx
	shr al, 4
	mov ah, 0x0C
	mov bh, 0 ; 页码
	int 10h
	inc cx ; 列
	pop bx

	mov al, byte[cs:bx + PIC]
	push bx
	and al, 0xF
	mov ah, 0x0C
	mov bh, 0 ; 页码
	int 10h
	inc cx ; 列
	pop bx


	inc bx ; 选择下两个像素, 两个像素一字节
	inc word [cs:DrawCount]
	inc word [cs:DrawXCount]

	cmp word [cs:DrawXCount], 16
	jne DCNOT_32

	cmp word [cs:DrawCount], 32*16
	je DRAW_END

	inc dx
	mov cx, [cs:DrawCol]
	mov word [cs:DrawXCount], 0
	DCNOT_32:

	jmp DRAWPOT
	DRAW_END:
	pop ax
	ret

DrawCount dw 0
DrawXCount dw 0
DrawCol dw 0

PIC:
%include "res.asm"
