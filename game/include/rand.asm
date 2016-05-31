rand:
	;return ax
	push dx
	mov ax, word [cs:RAND_SEED]
	mov dx, 109
	mul dx
	add ax, 71
	mov word[cs:RAND_SEED], ax
	pop dx
	ret

srand:
	push dx
	push cx
	push ax
	mov ax, 0x0200
	int 0x1a
	add cx, dx
	mov word[cs:RAND_SEED], cx
	pop ax
	pop cx
	pop dx
	ret

RAND_SEED dw 0
