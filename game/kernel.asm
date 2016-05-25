BITS 16
[global _start]
[extern main]

WinCol equ 20
WinRow equ 12
GridWidth equ 16
UpdateTimes equ 40

%include "keyboard.asm"
KEY_UP equ 0x4800
KEY_DOWN equ 0x5000
KEY_LEFT equ 0x4b00
KEY_RIGHT equ 0x4d00

;写入中断向量表
%macro WriteIVT 2
	mov ax,%1
	mov bx,4
	mul bx
	mov si,ax
	mov ax,%2
	mov [cs:si],ax ; offset
	mov ax,cs
	mov [cs:si + 2],ax
%endmacro

_start:
	mov ax, cs
	mov ds, ax
	mov ax, 0
	mov ss, ax
	mov sp, 7c00h

	;清屏
	mov ax, 3
	int 10h

	;视频模式
	;使用VGA 320x400 256色
	mov ah, 0
	mov al, 13h
	int 10h
	;使用SVGA, 640x480 256色
	;mov ax, 4F02H
	;mov bx, 101H
	;int 10h

	%include "color.asm"

	WriteIVT 08h,WKCNINTTimer ; Timer Interupt

	;SetTimer
	mov al,34h
	out 43h,al ; write control word
	mov ax,1193182/UpdateTimes	;X times / seconds
	out 40h,al
	mov al,ah
	out 40h,al

	sti

	jmp $

DrawMap:
	pusha

	mov word [cs:DrawRectW], 16
	mov word [cs:DrawRectH], 16

	mov si, 0
	mov cx, 0
	mov dx, 0

	DrawMapIn:

	mov al, byte [cs:MAP0 + si]
	mov ah, 0
	mov bx, GridWidth * GridWidth
	push dx
	mul bx
	pop dx
	add ax, MAPPIC
	mov bx, ax
	call DRAW

	add cx, GridWidth
	cmp cx, GridWidth * WinCol
	jne DrawMapNotGW
	mov cx, 0
	add dx, GridWidth
	DrawMapNotGW:
	inc si
	cmp si, WinRow * WinCol
	jne DrawMapIn

	popa
	ret

DrawPlayers:
	pusha
	HY_W equ 40
	HY_H equ 40
	mov word [cs:DrawRectW], HY_W
	mov word [cs:DrawRectH], HY_H
	mov bx, word [cs:(Players + _GRAPH_OFFSET)]
	mov ax, word [cs:(Players + _PAT_OFFSET)]
	mov cx, HY_W * HY_H 
	mul cx
	add bx, ax
	mov ax, word [cs:(Players + _DIR_OFFSET)]
	mov cx, HY_W * HY_H * 4 
	mul cx
	add bx, ax
	xor cx, cx
	mov cx, word [cs:(Players + _X_OFFSET)]	
	shr cx, 4
	mov dx, word [cs:(Players + _Y_OFFSET)] 	
	shr dx, 4
	sub cx, HY_W / 2
	sub dx, HY_H
	call DRAW

	popa
	ret

;Draw
;bx = offset PIC
;cx = column
;dx = row
DRAW:
	push ds
	push es
	pusha
	;[ds:si] -> [es:di]
	mov si, bx
	mov ax, cs
	mov ds, ax
	mov ax, 0A000h
	mov es, ax
	;设置di
	mov ax, dx
	mov bx, WinCol * GridWidth
	mul bx
	add ax, cx
	mov di, ax
	mov ax, word[cs:DrawRectH] ; 绘制行数
	DRAW_ONE_LINE:

	;mov cx, GridWidth / 2
	;rep movsw

	;绘制一行
	mov cx, word[cs:DrawRectW] ; 绘制列数
	MOVSWLOOP:
	mov bl, [ds:si]
	cmp bl, 11100011b ; opacity
	je IS_OPACITY
	mov [es:di], bl
	IS_OPACITY:
	inc si
	inc di
	loop MOVSWLOOP

	;换行
	sub di, word[cs:DrawRectW]
	add di, WinCol * GridWidth

	dec ax 
	jnz DRAW_ONE_LINE

	popa
	pop es
	pop ds
	ret

UpdatePlayers:
	pusha
	mov cx, word [cs:(Players + _V_OFFSET)]
	;CMP X
	mov ax, word [cs:(Players + _X_OFFSET)]
	mov bx, word [cs:(Players + _TX_OFFSET)]
	cmp ax, bx
	je XEQU
	ja XA 
	;X < TX
	mov word [cs:(Players + _DIR_OFFSET)], 2; turn right
	add word [cs:(Players + _X_OFFSET)], cx
	cmp word [cs:(Players + _X_OFFSET)], bx
	ja FIX_X
	jmp MovePlayer
	XA:
	;X > TX
	mov word [cs:(Players + _DIR_OFFSET)], 1; turn left	
	sub word [cs:(Players + _X_OFFSET)], cx
	cmp word [cs:(Players + _X_OFFSET)], bx
	jb FIX_X
	jmp MovePlayer
	XEQU:

	;CMP Y
	mov ax, word [cs:(Players + _Y_OFFSET)]
	mov bx, word [cs:(Players + _TY_OFFSET)]
	cmp ax, bx
	je YEQU
	ja YA 
	;Y < TY
	mov word [cs:(Players + _DIR_OFFSET)], 0; turn down
	add word [cs:(Players + _Y_OFFSET)], cx
	cmp word [cs:(Players + _Y_OFFSET)], bx
	ja FIX_Y
	jmp MovePlayer
	YA:
	;Y > TY
	mov word [cs:(Players + _DIR_OFFSET)], 3; turn up
	sub word [cs:(Players + _Y_OFFSET)], cx
	cmp word [cs:(Players + _Y_OFFSET)], bx
	jb FIX_Y
	jmp MovePlayer
	YEQU:

	;mov word [cs:(Players + _PAT_OFFSET)], 0
	cmp word [cs:(Players + _PAT_OFFSET)], 0
	je UpdatePlayerEND
	jmp MovePlayer

	FIX_X:
	mov word [cs:(Players + _X_OFFSET)], bx
	jmp MovePlayer

	FIX_Y:
	mov word [cs:(Players + _Y_OFFSET)], bx
	jmp MovePlayer

	MovePlayer:
    inc word [cs:(Players + _ANI_OFFSET)]
	cmp word [cs:(Players + _ANI_OFFSET)], 0x6
	jb UpdatePlayerEND
	mov word [cs:(Players + _ANI_OFFSET)], 0
	inc word [cs:(Players + _PAT_OFFSET)]
	and word [cs:(Players + _PAT_OFFSET)], 11b
	UpdatePlayerEND:
	popa
	ret

KeyJudge:
	pusha

	mov ah, 1
	int 16h
	jz KEYEND
	mov ah, 0
	int 16h

	PlayerVV equ 0x080

	mov cx, word [cs:(Players + _X_OFFSET)]
	mov dx, word [cs:(Players + _TX_OFFSET)]
	cmp cx, dx
	jne KEYEND

	mov cx, word [cs:(Players + _Y_OFFSET)]
	mov dx, word [cs:(Players + _TY_OFFSET)]
	cmp cx, dx
	jne KEYEND

	cmp ax, KEY_UP
	jne NextJudge2
	sub word[cs:(Players + _TY_OFFSET)], PlayerVV
	jmp KEYEND
	NextJudge2:
	cmp ax, KEY_DOWN
	jne NextJudge3
	add word[cs:(Players + _TY_OFFSET)], PlayerVV
	jmp KEYEND
	NextJudge3:
	cmp ax, KEY_LEFT
	jne NextJudge4
	sub word[cs:(Players + _TX_OFFSET)], PlayerVV
	jmp KEYEND
	NextJudge4:
	cmp ax, KEY_RIGHT
	jne KEYEND
	add word[cs:(Players + _TX_OFFSET)], PlayerVV
	jmp KEYEND

	KEYEND:
	popa
	ret

WKCNINTTimer:
	call KeyJudge
	call UpdatePlayers
	call DrawMap
	call DrawPlayers
	mov al,20h
	out 20h,al
	out 0A0h,al
	iret



DrawCount dw 0
DrawXCount dw 0
DrawCol dw 0
DrawMapCol dw 0
DrawRectW dw 16
DrawRectH dw 16


%macro SetOffset 1
	%1_OFFSET equ (%1 - Players)
%endmacro

SetOffset _GRAPH
SetOffset _DIR
SetOffset _PAT
SetOffset _X
SetOffset _Y
SetOffset _TX
SetOffset _TY
SetOffset _ANI
SetOffset _V

Players:
	_GRAPH dw G
	_DIR dw 0
	_PAT dw 0
	_X	dw 100h
	_Y	dw 100h
	_TX	dw 100h
	_TY	dw 100h
	_ANI dw 0
	_V	dw 0x20

MAPPIC:
%include "map256.asm"
G:
%include "g.asm"
MAP0:
%include "map0.asm"
