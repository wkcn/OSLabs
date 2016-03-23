[global _start]
[extern main]

_start:
	mov ax, cs
	mov ds, ax
	mov ax, 0xB800
	mov es, ax
	mov al,'T'
	mov ah,07h
	mov [es:2],ax
	jmp main
