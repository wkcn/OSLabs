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
	mov al, 13h
	int 10h

	mov dx, 0x3c6
	mov al,0xff
	out dx, al

	mov cl, 255

SetColor:
	mov dx, 0x3c8
	mov al, cl
	;Set Index
	out dx, al

	mov dx, 0x3c9
	
	mov al, cl
	shr al, 2
	and al, 0x38
	out dx, al
	
	mov al, cl
	shl al, 1
	and al, 0x38
	out dx, al

	mov al, cl
	shl al, 4
	and al, 0x30
	out dx, al

loop SetColor


	mov bx, PIC
	mov cx, 0
	mov dx, 0
	call DRAW

	mov bx, PIC + 32 * 32 * 5
	mov cx, 32
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

	mov al, byte[cs:bx]
	;mov al, byte[cs:ccc]
	inc byte[cs:ccc]
	push bx
	mov ah, 0x0C
	mov bh, 0 ; 页码
	int 10h
	inc cx ; 列
	pop bx

	inc bx ; 选择下两个像素, 两个像素一字节
	inc word [cs:DrawCount]
	inc word [cs:DrawXCount]

	cmp word [cs:DrawXCount], 32
	jne DCNOT_32

	cmp word [cs:DrawCount], 32*32
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
ccc db 0

PIC:
%include "map256.asm"
