;14348134 Operating System
;Jackie Wu (Wu Kan)
;14348134
;wkcn@live.cn

org 7c00h
UserProgramOffset equ 0A100h
;程序的编号,第一个和最后一个
PSTART	equ 1
PEND	equ 5

TextScreen equ 0B800h
outDelay equ 50000
inDelay equ 2400

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

WriteIVT 20h,WKCNINT20H
WriteIVT 21h,WKCNINT21H

%macro PrintColorStr 5 ; (message, msgLen, msgColor, x, y)
	mov ax,[%5]
	mov bx,80
	mul bx
	mov bx,[%4]
	add ax,bx
	mov di,ax

	mov dl,[%3]

	mov si, %1
	mov cx,[%2]
	printChar:
		mov al,[si]
		inc si
		mov ah,dl
		add dl,1
		mov [es:di],ax
		add di,2
	loop printChar	

	inc byte[%3]
%endmacro

%macro PrintStr 4 ;(message,msgLen,x,y) 
	push ax
	push bx
	push cx
	push dx
	push es
	push bp
	mov ax,cs
	mov es,ax
	mov ah,13h
	mov al,1
	mov bl,07h
	mov bh,0
	mov dh,%4 ;行
	mov dl,%3 ;列
	mov bp,%1
	mov cx,[%2]
	int 10h
	pop bp
	pop es
	pop dx
	pop cx
	pop bx
	pop ax
%endmacro

%macro LoadProgram 5  
	call CLEARSCREEN
	;设置段地址
	mov ax,cs
	mov es,ax
	;用户程序地址偏移
	mov bx,UserProgramOffset
	mov ah,2				  ;功能号
	mov al,%1				  ;扇区数
    mov dl,%2                 ;驱动器号 ; 软盘为0，硬盘和U盘为80H
    mov dh,%3                 ;磁头号 ; 起始编号为0
    mov ch,%4                 ;柱面号 ; 起始编号为0
    mov cl,%5                 ;起始扇区号 ; 起始编号为1
    int 13H ;                调用读磁盘BIOS的13h功能
	;执行用户程序
    jmp UserProgramOffset
%endmacro


START:
	;输出字符
	PrintStr msg,msgLen,0,0
	PrintStr msg2,msgLen2,0,1
	;获取键盘输入
	mov ah,00h
	int 16h
	mov bx,TextScreen 
	mov es,bx
	mov si,(1 * 80 + 38) * 2
	mov bl,al
	mov bh,07h
	mov [es:si],bx
	cmp al,PSTART + '0'
	jl START ; al < PSTART
	cmp al,PEND + '0'
	jg START ; al > PEND
	;valid
	sub al,'0'
	inc al ; 起始扇区号为1, 第一个用户程序在扇区2, 对应输入数字1
	mov [sectorNum],al
	LoadProgram 1,0,0,0,[sectorNum] ;扇区数, 驱动器号, 磁头号, 柱面号, 起始扇区号
jmp START

DELAY:
	mov cx, outDelay
	LOOP1:
		mov ax, inDelay	
		LOOP2:
			dec ax
			jg LOOP2
	loop LOOP1
	ret

CLEARSCREEN:
	mov ax,TextScreen
	mov es,ax
	mov si,0
	mov cx,80*25
	mov dx,0
	CLEARSCREENGO:
		mov [es:si],dx
		add si,2
	loop CLEARSCREENGO
	ret

WKCNINT20H:
	;input ctrl+z, and quit
	call PrintStr msg3,msgLen3,3,24
	mov ah,01h
	int 16h
	jz  NOCZ	;没有按键
	;按键了,获取字符
	mov ah,00h
	int 16h
	cmp ax,2c1ah
	jne NOCZ    ; 如果没有按Ctrl + Z, 跳转到NOCZ
	int 21h
	NOCZ:
	iret

WKCNINT21H:
	call 0000h:CLEARSCREEN
	jmp 0000h:START
	iret

DATA:
	msg	db	'Welcome to 14348134-OS by Jackie Wu (Wu Kan)'
	msgLen	dw	$ - msg

	msg2	db	'Input number (',PSTART + '0','~',PEND + '0',') to select program: '
	msgLen2	dw	$ - msg2

	msg3	db  'Input Ctrl + Z to return >.<'
	msgLen3 dw	$ - msg3

	sectorNum db 0

times 510-($-$$) db 0  
dw  0xaa55
