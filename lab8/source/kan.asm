;My Name
;Jackie Wu (Wu Kan)
;14348134

	org 100H
	;org 7c00h
	outDelay equ 40000
	inDelay equ 500

	;set data segment
	mov ax,cs
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

	SETMEM pos,1405H
	SETMEM vel,0FF01H
	mov cx,8
	call LINE

	SETMEM pos,1109H
	SETMEM vel,0001H
	mov cx,14
	call LINE

	SETMEM pos,1705H
	SETMEM vel,0001H
	mov cx,7
	call LINE

	SETMEM pos,1705H
	SETMEM vel,0100H
	mov cx,10
	call LINE

	SETMEM vel,0001H
	mov cx,7
	call LINE

	SETMEM pos,170BH
	SETMEM vel,0100H
	mov cx,10
	call LINE

	SETMEM pos,170DH
	SETMEM vel,0001H
	mov cx,8
	call LINE

	SETMEM pos,1615H
	SETMEM vel,0FF01H
	mov cx,2
	call LINE

	SETMEM pos,1B0DH
	SETMEM vel,0001H
	mov cx,10
	call LINE

	SETMEM pos,1F0DH
	SETMEM vel,0001H
	mov cx,9
	call LINE

	SETMEM vel,0100H
	mov cx,4
	call LINE

	SETMEM vel,00FFH
	mov cx,2
	call LINE

	call DELAY
	call DELAY
	call DELAY
	call DELAY
	call DELAY
	mov ax,3
	int 10h
	jmp START

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

	mov di, (1*80 + 1) * 2

	printChar:
		mov al,[si]
		inc si
		mov ah,07h
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
	inc al
	cmp al,'Z'
	jna LESSZ
	mov al,'A'
LESSZ:
	mov [char],al
	mov ah,[color]
	and ah,0Fh
	inc byte[color]
	mov [es:bx],ax
	pop cx
	ret

DATA:
	message db "My Name is"
	msgLen	dw $-message

	pos dw 0000h
	vel dw 0000h
	char db 'a'
	color db 09H

times 510-($-$$) db 0
dw 0xaa55
