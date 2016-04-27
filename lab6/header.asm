BITS 16

[global _start]
[extern main] 

_start:
	;mov ax, cs
	;mov ds, ax
	;mov ss, ax
	;mov sp, 100h - 4
	call main
	;发送程序结束信号,返回Shell
	sti
	int 20h
	;mov ax, 0x00
	;mov es, ax
	;mov ax, 0x7c00
	;mov si, ax
	;mov ax, 1
	;mov [es:si], ax
	jmp $
