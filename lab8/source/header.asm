BITS 16

[global _start]
[extern main] 

jmp _start

_start:
	push word 0 ; 由于G++的ret是32位的， 这里补0
	call main
	;调用int 20h
	;xor ax, ax
	;mov es, ax
	;mov di, 20h * 4
	;pushf
	;call far [es:di]
	sti
	int 20h
	jmp $
