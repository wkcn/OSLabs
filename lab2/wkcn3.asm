;Running Ball
;Jackie Wu (Wu Kan)
;14348134
;wkcn@live.cn

	;totalDelay = outDelay * inDelay
	org 0A100H
	outDelay equ 50000
	inDelay equ 2000

	;80 x 25
	SCREEN_X equ 80
	SCREEN_Y equ 25
	;Ball in this Rect
	MIN_X equ 0
	MIN_Y equ 13
	MAX_X equ 40
	MAX_Y equ 25

	;set data segment
	;mov ax,07c0h
	mov ax,cs
	mov ds,ax

	;text window
	;25 * 80
	mov ax,0B800h
	mov es,ax


%macro SINGLE 4
	;pos	dw	0000h	;from 0,0
	;vel	dw	0101h	;v = (1,1)
	;char db '*' 
	;color db 07H

	mov ax, [%1]
	mov [pos], ax 
	mov ax, [%2]
	mov [vel], ax 
	mov al, [%3]
	mov [char], al
	mov al, [%4]
	mov [color], al

	call PLAY

	mov ax, [pos]
	mov [%1], ax
	mov ax, [vel]
	mov [%2], ax
%endmacro

START:
	call DELAY
	;call PLAY
	SINGLE pos1,vel1,char1,color1
	SINGLE pos2,vel2,char2,color2
	SINGLE pos3,vel3,char3,color3
	call SHOWNAME
	int 20h
	jmp START

PLAY:
	call SETPOINTER
	call ELIMINATE

	call UPDATEPOS	

	call SETPOINTER
	call SHOW
	ret

SHOWNAME:
	mov cx,[msgLen]
	
	mov si,message

	mov di, (1*80 + 1) * 2

	mov dl,[msgColor]

	printChar:
		mov al,[si]
		inc si
		and dl,0Fh
		mov ah,dl
		add dl,1
		mov [es:di],ax
		add di,2
	loop printChar	

	inc byte[msgColor]

	ret

DELAY:
	mov cx, outDelay
	LOOP1:
		mov ax, inDelay	
		LOOP2:
			dec ax
			jg LOOP2
	loop LOOP1

	ret

UPDATEPOS:
	;parameter: pos, vel
	mov ax,[pos]
	mov bx,[vel]

	;update x
	add ah,bh
	cmp ah,MIN_X
	ja XNZ
	;if x <= MIN_X
	mov bh,1
XNZ:
	CMP ah,MAX_X-1
	jb	XNF
	;if x >= MAX_X-1
	mov bh,-1
XNF:
	;update y
	add al,bl
	cmp al,MIN_Y
	ja YNZ
	;if y <= MIN_Y
	mov bl,1
YNZ:
	CMP al,MAX_Y-1
	jb	YNF
	;if y >= MAX_Y-1
	mov bl,-1
YNF:
	mov [pos],ax
	mov [vel],bx
	ret

SETPOINTER:
	;parameter pos, char
	mov ax, 0
	mov bx, [pos] ; bx = (x,y)
	mov al, bl
	mov cx, SCREEN_X
	mul cx ; ax *= MAX_X namely ax = y * MAX_X
	mov cx, 0
	mov cl, bh
	add ax, cx ; ax += x 
	mov cx, 2
	mul cx
	mov bx, ax
	ret

SHOW:
	;new
	mov al, [char]
	mov ah, [color]
	mov [es:bx],ax
	ret

ELIMINATE:
	;clean
	mov ax, 0
	mov [es:bx],ax
	ret




DATA:
	message db "WuKan's Program 3"
	msgLen	dw $-message
	msgColor db 00h

	outCount dw outDelay
	inCount dw inDelay

	pos	dw	0000h	;from 0,0
	vel	dw	0101h	;v = (1,1)
	char db '*' 
	color db 03H

	;elements
	pos1 dw 0000h
	vel1 dw 0101h
	char1 db '*'
	color1 db 03H ;green 
	
	pos2 dw 1003h
	vel2 dw 01FFh
	char2 db 'A'
	color2 db 0CFH ;twinkle red and light white

	pos3 dw 2019h
	vel3 dw 0FFFFh
	char3 db '@'
	color3 db 0EH ;yello

times 510-($-$$) db 0  
dw  0xaa55
