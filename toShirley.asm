;To Shirley
;Jackie Wu (Wu Kan)
;2016-03-01

	outDelay equ 50000
	inDelay equ 2400

	;set data segment
	mov ax,07c0h
	mov ds,ax

	;text window
	;25 * 80
	mov ax,0B800h
	mov es,ax

%macro SETMEM 2
	mov ax,%2
	mov [%1],ax
%endmacro

START:
	call DELAY
	call SHOWINFO
	call DELAY

	SETMEM pos,1107H
	SETMEM vel,0001H
	mov cx,14
	call LINE

	SETMEM pos,1015H
	SETMEM vel,0FFFFH
	mov cx,2
	call LINE

	SETMEM pos,0F0CH
	SETMEM vel,0FF01H
	mov cx,5
	call LINE

	SETMEM pos,130CH
	SETMEM vel,0101H
	mov cx,5
	call LINE

	SETMEM pos,1D07H
	SETMEM vel,0100H
	mov cx,13
	call LINE

	SETMEM pos,1E0AH
	SETMEM vel,0001H
	mov cx,12
	call LINE

	SETMEM pos,1E0AH
	SETMEM vel,0100H
	mov cx,11
	call LINE

	SETMEM pos,280AH
	SETMEM vel,0001H
	mov cx,11
	call LINE

	SETMEM pos,2715H
	SETMEM vel,0FFFFH
	mov cx,2
	call LINE


	SETMEM pos,2307H
	SETMEM vel,0001H
	mov cx,15
	call LINE

	SETMEM pos,200DH
	SETMEM vel,0101H
	mov cx,2
	call LINE

	SETMEM pos,2011H
	SETMEM vel,0101H
	mov cx,2
	call LINE

	SETMEM pos,250DH
	SETMEM vel,0101H
	mov cx,2
	call LINE
	
	SETMEM pos,2511H
	SETMEM vel,0101H
	mov cx,2
	call LINE

	jmp $

LINE:
	;cx is the len of line
	LINELOOP:
		call DELAY
		call SHOW
		call MOVEPOS
	loop LINELOOP
	ret

SHOWINFO:
	mov cx,[msgLen]
	
	mov si,message

	mov di, 0*80 + 0

	printChar:
		mov al,[si]
		inc si
		mov ah,07H
		mov [es:di],ax
		add di,2
	loop printChar	
	ret

DELAY:
	push cx
	mov cx, outDelay
	LOOP1:
		mov ax, inDelay	
		LOOP2:
			dec ax
			jg LOOP2
	loop LOOP1
	pop cx

	ret

MOVEPOS:
	mov ax,[pos]
	mov bx,[vel]
	add ah,bh
	add al,bl
	mov [pos],ax
	ret

SHOW:
	push cx
	mov bx,[pos]
	mov ax,0
	mov al,bl
	mov dx,80
	mul dx
	mov dx,0
	mov dl,bh
	add ax,dx
	mov cx,2
	mul cx
	mov bx, ax

	mov al,[char]
	mov ah,[color]
	mov [es:bx],ax
	pop cx
	ret

DATA:
	message db "To Shirley"
	msgLen	dw $-message

	pos dw 0000h
	vel dw 0000h
	char db 'a'
	color db 09H

times 510-($-$$) db 0
dw 0xaa55
