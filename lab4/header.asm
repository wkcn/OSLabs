BITS 16

[global _start]
[extern main] 

_start:
	mov ax, cs
	mov ds, ax
	push word NEXT
	jmp main
NEXT:
	mov ax, 0x00
	mov es, ax
	mov ax, 0x7c00
	mov si, ax
	mov ax, 1
	mov [es:si], ax
	jmp $
