;设置调色板

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
