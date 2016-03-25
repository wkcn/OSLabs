BITS 16
[global _start]
[extern main]

[global _RunProg]
UserProgramOffset equ 0A100h

_start:
	mov ax, cs
	mov ds, ax
	mov ax, 0xB800
	mov es, ax
	mov al,'T'
	mov ah,07h
	mov [es:2],ax
	jmp main


%macro LoadProgram 5  
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

_RunProg:
	LoadProgram 1,0,0,0,2
