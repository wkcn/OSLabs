BITS 16

[global _start]
[extern main] 

_start:
	mov ax, cs
	mov ds, ax
	call main
	int 20h
	;mov ax, 0x00
	;mov es, ax
	;mov ax, 0x7c00
	;mov si, ax
	;mov ax, 1
	;mov [es:si], ax
	jmp $
