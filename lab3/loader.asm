BITS 16
org 7c00h
OS_OFFSET equ 7e00h

Start:
	mov ax,cs
	mov ds,ax
	mov es,ax

ReadOS:
	;OS OFFSET
	mov bx, OS_OFFSET
	mov ah, 2 ; kind of function
	mov al, 8 ; read num of shanqu
	mov dl, 0 ; floppy
	mov dh, 0 ; citou
	mov ch, 0 ; zhumian
	mov cl, 3 ; start_shanqu
	int 13h

JUMP_TO_OS:
	jmp 0:OS_OFFSET

times 510 - ($ - $$) db 0
db 0x55
db 0xaa
