BITS 16
[global _start]
[extern main]
[extern AI]
[extern Update]


[global Players]
[global Bombs]
[global Powers]
[global PASSED_DATA]
[global STATE_DATA]


[global PlayerHP]
[global PlayerNOHARM]
[global BossHP]
[global BossNOHARM]

WinCol equ 20
WinRow equ 12
GridWidth equ 16
UpdateTimes equ 60

VideoBuffer equ 0x8000

MAPPIC0_SEG equ 0x4000
HUOYING_SEG equ MAPPIC0_SEG + 0x800 
PEOPLE_SEG equ HUOYING_SEG + 0x800
FOOTBALL_SEG equ PEOPLE_SEG + 0x200
BOSS_SEG equ FOOTBALL_SEG + 0x200
POWER_SEG equ BOSS_SEG + 0x800 
WIN_SEG equ POWER_SEG + 0x200 
LOSE_SEG equ WIN_SEG + 0x200
TITLE_SEG equ LOSE_SEG + 0x200

BossMaxHP equ 5

PowerTime equ 1
BombTime equ 4
FootballTime equ 3
FootballIncTime equ 9
TQP equ 5 ; 踢球最大距离

;state
BombFlag equ 0x01
PowerFlag equ 0x02


TotalBomb equ 20 
TotalPower equ WinCol * WinRow

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

%macro LoadFile 3
	;Name, Segment, Offset
	;NOte, num, num
	;ex: LoadFile Miku, 39, 0
	mov word[cs:FileNameP], %1
	mov word[cs:FileSegment], %2
	mov word[cs:FileOffset], %3
	call ReadFloppy
%endmacro

_start:
	mov ax, cs
	mov ds, ax
	mov ss, ax
	mov sp, 100h

	;清屏
	mov ax, 3
	int 10h

	;视频模式
	;使用VGA 320x200 256色
	mov ah, 0
	mov al, 13h
	int 10h
	;使用SVGA, 640x480 256色
	;mov ax, 4F02H
	;mov bx, 101H
	;int 10h

	%include "color.asm"

	;Resource
	LoadFile MAPPIC0, MAPPIC0_SEG, 0
	LoadFile HUOYING, HUOYING_SEG, 0
	LoadFile FOOTBALL, FOOTBALL_SEG, 0
	LoadFile PEOPLE, PEOPLE_SEG, 0
	LoadFile BOSSName, BOSS_SEG, 0
	LoadFile PowerName, POWER_SEG, 0
	LoadFile WinName, WIN_SEG, 0
	LoadFile LoseName, LOSE_SEG, 0
	LoadFile TitleName, TITLE_SEG, 0

	call srand

	;SetTimer
	mov al,34h
	out 43h,al ; write control word
	mov ax,1193182/UpdateTimes	;X times / seconds
	out 40h,al
	mov al,ah
	out 40h,al

	;当这个被执行时， 时钟中断会马上开始！
	WriteIVT 08h,WKCNINTTimer ; Timer Interupt

	sti

	;重启游戏
	REFRESH_GAME:

	;显示游戏标题
	mov word[cs:DrawRectW], 320 
	mov word[cs:DrawRectH], 200
	mov word[cs:DrawSegment], TITLE_SEG
	mov bx, 0
	mov cx, 0
	mov dx, 0 
	call DRAW
	call UpdateScreen

	;等待任意按键以开始游戏
	mov ah, 0
	int 16h
	call StartGame


	;在游戏停止时判断是否按下空格退回标题界面
	TEST_PRESS:
	cmp byte[cs:IsStartGame], 0
	jne TEST_PRESS
	call CheckPress
	jmp REFRESH_GAME

	jmp $

;FAT12文件读取
%include "disk.asm"
;伪随机数生成
%include "rand.asm"

;检查是否按下空格（等待）
CheckPress:
	CheckP:
	mov ah, 0
	int 16h
	cmp ax, KEY_SPACEBAR
	jne CheckP
	ret

StartGame:
	;设置参数
	mov si, Players
	mov word[cs:si + _X_OFFSET], 700h
	mov word[cs:si + _Y_OFFSET], 600h
	mov word[cs:si + _TX_OFFSET], 700h
	mov word[cs:si + _TY_OFFSET], 600h
	mov si, BOSS
	mov word[cs:si + _X_OFFSET], 0b00h
	mov word[cs:si + _Y_OFFSET], 600h
	mov word[cs:si + _TX_OFFSET], 0b00h
	mov word[cs:si + _TY_OFFSET], 600h

	mov si, Bombs
	mov cx, TotalBomb

	ClearBombs:
		mov byte[cs:(si + _USED_B_OFFSET)], 0
		add si, BombDataSize
	loop ClearBombs

	mov si, Powers
	mov cx, TotalPower
	ClearPowers:
		mov byte[cs:si + _USED_P_OFFSET], 0
		add si, PowerSize
	loop ClearPowers

	mov si, STATE_DATA
	mov cx, WinRow * WinCol
	ClearState:
		mov byte[cs:si], 0
		inc si
	loop ClearState

	mov byte[cs:PlayerHP], 1
	mov byte[cs:BossHP], BossMaxHP

	mov word[cs:FootBallTC], FootballTime * UpdateTimes
	mov byte[cs:FootBallNum], 0
	mov byte[cs:FootBallMaxNum], 3
	mov word[cs:FootBallIncCount], FootballIncTime * UpdateTimes


	mov byte[cs:IsStartGame], 1
	ret

;绘制地图
DrawMap:
	pusha

	mov word [cs:DrawRectW], GridWidth
	mov word [cs:DrawRectH], GridWidth

	mov word [cs:DrawSegment], MAPPIC0_SEG
	mov si, MAP0
	call DrawMapLayer

	mov word [cs:DrawSegment], PEOPLE_SEG
	mov si, MAP3
	call DrawMapLayer

	popa
	ret

;绘制某层地图
DrawMapLayer:

	pusha 

	mov word [cs:DrawRectW], GridWidth
	mov word [cs:DrawRectH], GridWidth

	mov di, 0
	mov cx, 0
	mov dx, 0

	DrawMapIn:

	mov al, byte [cs:si]
	cmp al, 0
	je DRAWEND
	mov ah, 0
	mov bx, GridWidth * GridWidth
	push dx
	mul bx
	pop dx
	;add ax, MAPPIC - GridWidth * GridWidth
	sub ax, GridWidth * GridWidth
	mov bx, ax
	call DRAW

	DRAWEND:
	add cx, GridWidth
	cmp cx, GridWidth * WinCol
	jne DrawMapNotGW
	;换行
	mov cx, 0
	add dx, GridWidth
	DrawMapNotGW:
	inc si
	inc di
	cmp di, WinRow * WinCol

	jne DrawMapIn

	popa

	ret


DrawHP:
	;320 * 200
	;8像素
	mov ax, VideoBuffer
	mov es, ax

	mov bx, 320 * 192
	mov cx, 320 * 8
	DRAWBUTTOM:
		mov byte[es:bx], 00000001b
		inc bx
	loop DRAWBUTTOM

	mov ax, 320 / BossMaxHP
	mov bl, byte[cs:BossHP]
	mul bl

	;不知道为什么，显示在最下方会有刷新问题
	mov cx, 4
	mov bx, 0;320 * 192

	cmp ax, 0
	je DRAWHPEND

	DrawHPIN:
		push ax
		push bx
		push cx

		mov cx, 320

		HPPot:	
			mov byte[es:bx], 11100000b
			inc bx
			dec cx
			dec ax
		jnz HPPot

		jcxz HPBE
		HPB:
			mov byte[es:bx], 00100000b
			inc bx
		loop HPB

		HPBE:
		pop cx
		pop bx
		pop ax
		add bx, 320
	loop DrawHPIN
	DRAWHPEND:
	ret

DrawPlayer:
	pusha
	HY_W equ 40
	HY_H equ 40
	mov word [cs:DrawRectW], HY_W
	mov word [cs:DrawRectH], HY_H
	mov word [cs:DrawSegment], HUOYING_SEG

	mov ax, word [cs:(si + _GRAPH_OFFSET)]
	mov word [cs:DrawSegment], ax

	mov bx, 0
	mov ax, word [cs:(si + _PAT_OFFSET)]
	mov cx, HY_W * HY_H 
	mul cx
	add bx, ax
	mov ax, word [cs:(si + _DIR_OFFSET)]
	mov cx, HY_W * HY_H * 4 
	mul cx
	add bx, ax
	xor cx, cx
	mov cx, word [cs:(si + _X_OFFSET)]	
	shr cx, 4
	mov dx, word [cs:(si + _Y_OFFSET)] 	
	shr dx, 4
	;sub cx, HY_W / 2
	;sub dx, HY_H
	sub cx, - (GridWidth - HY_W) / 2
	add dx, (GridWidth - HY_H)
	call DRAW

	popa
	ret

DrawBomb:
	pusha
	BombSize equ 18
	mov word [cs:DrawRectW], BombSize
	mov word [cs:DrawRectH], BombSize

	mov ax, word [cs:(si + _GRAPH_B_OFFSET)]
	mov word [cs:DrawSegment], ax

	mov bx, 0
	mov ax, word [cs:(si + _PAT_B_OFFSET)]
	mov cx, BombSize * BombSize
	mul cx
	add bx, ax
	xor cx, cx
	mov cx, word [cs:(si + _X_B_OFFSET)]	
	shr cx, 4
	mov dx, word [cs:(si + _Y_B_OFFSET)] 	
	shr dx, 4
	;sub cx, BombSize / 2
	;sub dx, BombSize
	sub cx, - (GridWidth - BombSize) / 2
	add dx, (GridWidth - BombSize)
	call DRAW

	popa
	ret


DrawPower:
	pusha
	mov word [cs:DrawRectW], GridWidth
	mov word [cs:DrawRectH], GridWidth

	mov word [cs:DrawSegment], POWER_SEG

	mov bx, 0
	mov ax, word [cs:(si + _PAT_P_OFFSET)]
	mov cx, GridWidth * GridWidth
	mul cx
	add bx, ax
	xor cx, cx
	mov cx, word [cs:(si + _X_P_OFFSET)]	
	shr cx, 4
	mov dx, word [cs:(si + _Y_P_OFFSET)] 	
	shr dx, 4
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
	mov ax, word[cs:DrawSegment]
	mov ds, ax
	mov ax, VideoBuffer
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

UpdatePlayer:
	pusha
	mov cx, word [cs:(si + _V_OFFSET)]
	;CMP X
	mov ax, word [cs:(si + _X_OFFSET)]
	mov bx, word [cs:(si + _TX_OFFSET)]
	cmp ax, bx
	je XEQU
	ja XA 
	;X < TX
	mov word [cs:(si + _DIR_OFFSET)], 2; turn right
	add word [cs:(si + _X_OFFSET)], cx
	cmp word [cs:(si + _X_OFFSET)], bx
	ja FIX_X
	jmp MovePlayer
	XA:
	;X > TX
	mov word [cs:(si + _DIR_OFFSET)], 1; turn left	
	sub word [cs:(si + _X_OFFSET)], cx
	cmp word [cs:(si + _X_OFFSET)], bx
	jb FIX_X
	jmp MovePlayer
	XEQU:

	;CMP Y
	mov ax, word [cs:(si + _Y_OFFSET)]
	mov bx, word [cs:(si + _TY_OFFSET)]
	cmp ax, bx
	je YEQU
	ja YA 
	;Y < TY
	mov word [cs:(si + _DIR_OFFSET)], 0; turn down
	add word [cs:(si + _Y_OFFSET)], cx
	cmp word [cs:(si + _Y_OFFSET)], bx
	ja FIX_Y
	jmp MovePlayer
	YA:
	;Y > TY
	mov word [cs:(si + _DIR_OFFSET)], 3; turn up
	sub word [cs:(si + _Y_OFFSET)], cx
	cmp word [cs:(si + _Y_OFFSET)], bx
	jb FIX_Y
	jmp MovePlayer
	YEQU:

	;mov word [cs:(si + _PAT_OFFSET)], 0
	cmp word [cs:(si + _PAT_OFFSET)], 0
	je UpdatePlayerEND
	jmp MovePlayer

	FIX_X:
	mov word [cs:(si + _X_OFFSET)], bx
	jmp MovePlayer

	FIX_Y:
	mov word [cs:(si + _Y_OFFSET)], bx
	jmp MovePlayer

	MovePlayer:
    inc word [cs:(si + _ANI_OFFSET)]
	cmp word [cs:(si + _ANI_OFFSET)], 6
	jb UpdatePlayerEND
	mov word [cs:(si + _ANI_OFFSET)], 0
	inc word [cs:(si + _PAT_OFFSET)]
	and word [cs:(si + _PAT_OFFSET)], 11b
	UpdatePlayerEND:
	popa
	ret

FindEmptyPower:
	;return di
	mov di, Powers
	FindPowerIn:
		cmp byte [cs:(di + _USED_P_OFFSET)], 0
		je FoundPower
		add di, PowerSize
	jmp FindPowerIn
	FoundPower:
	mov byte [cs:(di + _USED_P_OFFSET)], 1
	ret

FindEmptyBomb:
	;return di
	mov di, Bombs
	FindBombIn:
		cmp byte [cs:(di + _USED_B_OFFSET)], 0
		je FoundBomb
		add di, BombDataSize
	jmp FindBombIn
	FoundBomb:
	mov byte [cs:(di + _USED_B_OFFSET)], 1
	ret

OldPowerX dw 0
OldPowerY dw 0
BombPower db 0
BombFirst db 0

UpdateBomb:
	pusha
	;爆炸判断


	mov cx, word [cs:(si + _X_B_OFFSET)]
	mov dx, word [cs:(si + _Y_B_OFFSET)]
	shr cx, 8
	shr dx, 8

	mov ax, dx
	mov dx, WinCol
	mul dx
	add ax, cx
	mov bx, ax
	cmp byte[cs:STATE_DATA + bx], PowerFlag
	jne NoPowerTrigger
	cmp word [cs:(si + _COUNT_B_OFFSET)], 1
	jle NoPowerTrigger
	mov word [cs:(si + _COUNT_B_OFFSET)], 1
	NoPowerTrigger:

	dec word [cs:(si + _COUNT_B_OFFSET)]
	jnz MOVEJUDGE

	mov byte [cs:(si + _USED_B_OFFSET)], 0
	;Bombed
	%macro Bombing 3
		mov cx, word [cs:(si + _X_B_OFFSET)]
		mov dx, word [cs:(si + _Y_B_OFFSET)]
		;以下两句不必要
		;add cx, 0x80
		;add dx, 0x80
		shr cx, 8
		shr dx, 8
		mov bl, byte [cs:(si + _POWER_B_OFFSET)]
		mov byte [cs:(BombPower)], bl
		;mov word [cs:OldPowerX], cx
		;mov word [cs:OldPowerY], dx
		mov ax, cx
		mov bx, dx
		add cx, %1
		add dx, %2
		;mov byte[cs:BombFirst], 1
		BombingIn_%3:
			;mov word [cs:OldPowerX], cx
			;mov word [cs:OldPowerY], dx
			;add cx, %1
			;add dx, %2
			mov ax, cx
			mov bx, dx
			add cx, %1
			add dx, %2
			call IsPassed	
			jz THROUGH_%3

			;cmp byte [cs:BombFirst], 1
			;je FinishBombing_%3
			push cx
			push dx
			mov cx, ax
			mov dx, bx
			call IsPassed
			jz CanPutPower_%3
			pop dx
			pop cx
			jmp FinishBombing_%3

			CanPutPower_%3:
			pop dx
			pop cx

			call FindEmptyPower 
			;不穿透情况
			mov word [cs:(di + _PAT_P_OFFSET)], %3
			mov byte [cs:BombPower], 1
			jmp SetPowerPated_%3

			THROUGH_%3:
			call FindEmptyPower 
			;穿透情况
			cmp byte[cs:BombPower], 1
			jle PowerEnd_%3

			mov word [cs:(di + _PAT_P_OFFSET)], %3 + 4
			jmp SetPowerPated_%3

			PowerEnd_%3:
			mov word [cs:(di + _PAT_P_OFFSET)], %3



			SetPowerPated_%3:
			push bx
			push ax
			shl ax, 8
			shl bx, 8
			mov word [cs:(di + _X_P_OFFSET)], ax
			mov word [cs:(di + _Y_P_OFFSET)], bx
			mov word [cs:(di + _COUNT_P_OFFSET)], PowerTime * UpdateTimes
			pop ax
			pop bx

			
			push dx
			push cx
			push bx
			push ax

			mov cx, ax
			mov ax, bx
			mov bx, WinCol
			mul bx
			add ax, cx
			mov bx, ax
			mov byte [cs:STATE_DATA + bx], PowerFlag

			pop ax
			pop bx
			pop cx
			pop dx



		;mov byte[cs:BombFirst], 0
		dec byte [cs:BombPower]
		jnz BombingIn_%3
		FinishBombing_%3:
	%endmacro

	;center
	mov cx, word [cs:(si + _X_B_OFFSET)]
	mov dx, word [cs:(si + _Y_B_OFFSET)]
	add cx, 0x80
	add dx, 0x80
	and cx, 0xFF00
	and dx, 0xFF00
	call FindEmptyPower
	mov word [cs:(di + _PAT_P_OFFSET)], 8
	mov word [cs:(di + _X_P_OFFSET)], cx
	mov word [cs:(di + _Y_P_OFFSET)], dx
	mov word [cs:(di + _COUNT_P_OFFSET)], PowerTime * UpdateTimes

	push dx
	push cx
	push bx
	push ax

	shr cx, 8
	shr dx, 8
	mov ax, dx
	mov dx, WinCol
	mul dx
	add ax, cx
	mov bx, ax
	;原地会将原来的BombFlag -> PowerFlag
	mov byte[cs:STATE_DATA + bx], PowerFlag

	pop ax
	pop bx
	pop cx
	pop dx

	Bombing 0,1,0
	Bombing -1,0,1
	Bombing 1,0,2
	Bombing 0,-1,3

	MOVEJUDGE:
		

	UpdateBombAni:
	;动画刷新
    inc word [cs:(si + _ANI_B_OFFSET)]
	cmp word [cs:(si + _ANI_B_OFFSET)], 6
	jb UpdateBombEND
	mov word [cs:(si + _ANI_B_OFFSET)], 0
	inc word [cs:(si + _PAT_B_OFFSET)]
	and word [cs:(si + _PAT_B_OFFSET)], 11b

	;移动判断
	cmp word [cs:(si + _JUMP_COUNT_B_OFFSET)], 0
	jle UpdateBombEND 

	dec word [cs:si + _JUMP_COUNT_B_OFFSET]
	jnz JumpCountNotZero
	;跳跃结束时， 设置FLAG
	mov cx, word [cs:si + _TX_B_OFFSET]
	mov dx, word [cs:si + _TY_B_OFFSET]
	shr cx, 8
	shr dx, 8
	mov ax, dx
	mov dx, WinCol
	mul dx
	add ax, cx
	mov bx, ax
	mov byte[cs:STATE_DATA + bx], BombFlag
	JumpCountNotZero:

	;跳跃处理
	mov cx, word [cs:(si + _X_B_OFFSET)]
	mov bx, word [cs:si + _JUMP_COUNT_B_OFFSET]
	mov ax, bx
	; ax = jump_count
	; cx = x
	mul cx
	add ax, word [cs:si + _TX_B_OFFSET]
	adc dx, 0
	inc bx
	div bx
	mov word[cs:si + _X_B_OFFSET], ax


	mov cx, word [cs:(si + _Y_B_OFFSET)]
	mov bx, word [cs:si + _JUMP_COUNT_B_OFFSET]
	mov ax, bx
	mul cx
	add ax, word [cs:si + _TY_B_OFFSET]
	adc dx, 0
	inc bx
	div bx
	mov word[cs:si + _Y_B_OFFSET], ax

	UpdateBombEND:

	popa
	ret

TQAX dw 0
TQAY dw 0
FBInd dw 0
FBpos dw 0

TQIN:
	;首先判断(cx,dx)是否有足球
	push dx
	push cx
	mov ax, dx
	mov dx, WinCol
	mul dx
	add ax, cx
	mov bx, ax
	cmp byte [cs:STATE_DATA + bx], BombFlag
	jne NOFBIN
	;存在足球
	;mov word [cs:FBpos], bx; 保存位置
	mov byte [cs:STATE_DATA + bx], 0
	pop cx
	pop dx
	jmp SelectFBEnd

	;要特别注意这种压栈后跳出的情况
	NOFBIN:
	pop cx
	pop dx
	jmp NOFB

	SelectFBEnd:

	mov si, Bombs
	mov ax, cx
	mov cx, TotalBomb
	FindFB:
		cmp byte[cs:si + _USED_B_OFFSET], 1
		jne NextFB
		cmp byte[cs:si + _X_B_OFFSET + 1], al
		jne NextFB
		cmp byte[cs:si + _Y_B_OFFSET + 1], dl
		jne NextFB
		jmp FoundFB
		NextFB:
		add si, BombDataSize
	loop FindFB
	FoundFB:
	;si is football	
	mov cx, ax
	mov ax, TQP
	mov word [cs:si + _JUMP_COUNT_B_OFFSET], 1
	TT:
		add cx, word[cs:TQAX]
		add dx, word[cs:TQAY]
		call IsPassedPlayer
		jne TTNEXT
		push cx
		push dx
		shl cx, 8
		shl dx, 8
		mov word [cs:si + _TX_B_OFFSET], cx
		mov word [cs:si + _TY_B_OFFSET], dx
		inc word [cs:si + _JUMP_COUNT_B_OFFSET]
		pop dx
		pop cx
		TTNEXT:
		dec ax
	jnz TT
	TTEND:

	NOFB:
	ret

%macro TQ 2
	pusha
	mov word[cs:TQAX], %1
	mov word[cs:TQAY], %2
	call TQIN
	popa
%endmacro

KeyJudge:
	pusha

	mov ah, 1
	int 16h
	jz KEYEND
	mov ah, 0
	int 16h

	PlayerVV equ 0x080

	mov cx, word [cs:(si + _X_OFFSET)]
	mov dx, word [cs:(si + _TX_OFFSET)]
	cmp cx, dx
	jne KEYEND

	mov cx, word [cs:(si + _Y_OFFSET)]
	mov dx, word [cs:(si + _TY_OFFSET)]
	cmp cx, dx
	jne KEYEND


	mov cx, word [cs:(si + _X_OFFSET)]
	mov dx, word [cs:(si + _Y_OFFSET)]

	add cx, 0x80
	add dx, 0x80
	shr cx, 8
	shr dx, 8

	cmp ax, KEY_UP
	jne NextJudge2

	mov word[cs:(si + _DIR_OFFSET)], 3
	dec dx
	TQ 0,-1
	call IsPassedPlayer
	jne YBLOCK
	sub word[cs:(si + _TY_OFFSET)], PlayerVV

	jmp KEYEND
	NextJudge2:
	cmp ax, KEY_DOWN
	jne NextJudge3


	mov word[cs:(si + _DIR_OFFSET)], 0
	inc dx
	TQ 0,1
	call IsPassedPlayer
	jne YBLOCK
	add word[cs:(si + _TY_OFFSET)], PlayerVV

	jmp KEYEND
	NextJudge3:
	cmp ax, KEY_LEFT
	jne NextJudge4

	mov word[cs:(si + _DIR_OFFSET)], 1
	dec cx
	TQ -1,0
	call IsPassedPlayer
	jne XBLOCK
	sub word[cs:(si + _TX_OFFSET)], PlayerVV

	jmp KEYEND
	NextJudge4:
	cmp ax, KEY_RIGHT
	jne NextJudge5


	mov word[cs:(si + _DIR_OFFSET)], 2
	inc cx
	TQ 1,0
	call IsPassedPlayer
	jne XBLOCK
	add word[cs:(si + _TX_OFFSET)], PlayerVV

	jmp KEYEND

	NextJudge5:
	jmp KEYEND ; 暂时不允许角色自己放泡泡
	cmp ax, KEY_SPACEBAR
	jne KEYEND

	;mov byte[cs:BossHP], 3

	mov cx, word [cs:(si + _X_OFFSET)]
	mov dx, word [cs:(si + _Y_OFFSET)]
	add cx, 0x80
	and cx, 0xFF00
	add dx, 0x80
	and dx, 0xFF00
	call FindEmptyBomb

	;_GRAPH_B dw FOOTBALL_SEG
	;_USED_B db 1
	;_POWER_B db 3
	;_COUNT_B dw 5 * UpdateTimes
	;_PAT_B dw 0
	;_X_B	dw 000h
	;_Y_B	dw 000h
	;_TX_B	dw 000h
	;_TY_B	dw 000h
	;_ANI_B dw 0
	;_V_B	dw 0x20
	mov word[cs:(di + _GRAPH_B_OFFSET)], FOOTBALL_SEG
	mov byte[cs:(di + _USED_B_OFFSET)], 1
	mov byte[cs:(di + _POWER_B_OFFSET)], 3
	mov word[cs:(di + _COUNT_B_OFFSET)], BombTime * UpdateTimes
	mov word[cs:(di + _PAT_B_OFFSET)], 0
	mov word[cs:(di + _X_B_OFFSET)], cx
	mov word[cs:(di + _Y_B_OFFSET)], dx
	mov word[cs:(di + _TX_B_OFFSET)], cx
	mov word[cs:(di + _TY_B_OFFSET)], dx
	mov word[cs:(di + _ANI_B_OFFSET)], 0

	shr cx, 8
	shr dx, 8
	mov ax, dx
	mov dx, WinCol
	mul dx
	add ax, cx
	mov bx, ax
	mov byte[cs:STATE_DATA + bx], BombFlag

	jmp KEYEND

	XBLOCK:
	mov cx, word [cs:(si + _TX_OFFSET)]
	add cx, 0x80
	shr cx, 8
	shl cx, 8
	mov word [cs:(si + _TX_OFFSET)], cx
	jmp KEYEND

	YBLOCK:
	mov cx, word [cs:(si + _TY_OFFSET)]
	add cx, 0x80
	shr cx, 8
	shl cx, 8
	mov word [cs:(si + _TY_OFFSET)], cx

	KEYEND:
	popa
	ret

UpdateScreen:
	push es
	push ds
	push si
	push di
	push ax
	;[ds:si] -> [es:di]
	mov ax, VideoBuffer
	mov ds, ax
	mov ax, 0A000h
	mov es, ax
	mov si, 0
	mov di, 0
	mov cx, 320 * 200 / 2;WinCol * WinRow * GridWidth * GridWidth/ 2
	rep movsw
	pop ax
	pop di
	pop si
	pop ds
	pop es
	ret

IsPassed:
	;cx: column
	;dx: row
	;passed = je = zf
	;Check Border
	;类似检测数组越界, 用无符号判定
	push dx
	push cx
	push bx
	push ax
	cmp cx, WinCol
	jae NotPassed
	cmp dx, WinRow
	jae NotPassed
	mov ax, dx
	mov dx, WinCol
	mul dx
	add ax, cx
	mov bx, ax

	cmp byte[cs:PASSED_DATA + bx], 0
	jmp IsPassedEnd

	NotPassed:
	;设置ZF = 0
	mov bx, 0
	cmp bx, 1
	IsPassedEnd:
	pop ax
	pop bx
	pop cx
	pop dx
	ret

IsPassedPlayer:
	;cx: column
	;dx: row
	;passed = je = zf
	;Check Border
	;类似检测数组越界, 用无符号判定
	push dx
	push cx
	push bx
	push ax
	cmp cx, WinCol
	jae NotPassedPlayer
	cmp dx, WinRow
	jae NotPassedPlayer
	mov ax, dx
	mov dx, WinCol
	mul dx
	add ax, cx
	mov bx, ax

	cmp byte[cs:STATE_DATA + bx], BombFlag
	je NotPassedPlayer ; 不能走上炸弹
	cmp byte[cs:PASSED_DATA + bx], 0
	jmp IsPassedEndPlayer

	NotPassedPlayer:
	;设置ZF = 0
	mov bx, 0
	cmp bx, 1
	IsPassedEndPlayer:
	pop ax
	pop bx
	pop cx
	pop dx
	ret

UpdateFootball:
	;Football

	cmp byte[cs:FootBallMaxNum], TotalBomb
	je NoFBInc
	dec word[cs:FootBallIncCount]
	jnz NoFBInc
	mov word[cs:FootBallIncCount], FootballIncTime * UpdateTimes
	inc byte[cs:FootBallMaxNum]
	NoFBInc:

	dec word[cs:FootBallTC]
	jnz NoFootBall
	mov word[cs:FootBallTC], FootballTime * UpdateTimes
	mov al, byte [cs:FootBallMaxNum]
	mov byte[cs:FootBallNum], al

	PutFB:

	;不知道为什么，除法溢出时会卡住
	xor dx, dx
	call rand
	mov bx, WinCol
	div bx
	mov cx, dx

	xor dx, dx
	call rand
	mov bx, WinRow
	div bx

	call IsPassedPlayer
	jne PutFB

	cmp cx, WinCol / 2
	jge FBRight
	mov ax, 0
	jmp FBLR
	FBRight:
	mov ax, WinCol - 1
	shl ax, 8

	FBLR:

	;Put
	;jump count = delta x + delta y

	shl cx, 8
	shl dx, 8


	call FindEmptyBomb
	mov word[cs:(di + _GRAPH_B_OFFSET)], FOOTBALL_SEG
	mov byte[cs:(di + _POWER_B_OFFSET)], 3
	mov word[cs:(di + _COUNT_B_OFFSET)], BombTime * UpdateTimes
	mov word[cs:(di + _PAT_B_OFFSET)], 0
	mov word[cs:(di + _X_B_OFFSET)], ax
	mov word[cs:(di + _TX_B_OFFSET)], cx
	mov word[cs:(di + _TY_B_OFFSET)], dx
	mov word[cs:(di + _ANI_B_OFFSET)], 0


	call rand
	xor dx, dx
	mov bx, WinRow
	div bx
	shl dx, 8
	mov word[cs:(di + _Y_B_OFFSET)], dx

	;compute jump count
	mov cx, word [cs:di + _X_B_OFFSET]
	cmp cx, word [cs:di + _TX_B_OFFSET]
	jg XGreaterTX
	mov cx, word [cs:di + _TX_OFFSET]
	sub cx, word [cs:di + _X_OFFSET]
	jmp NextXTX
	XGreaterTX:
	sub cx, word [cs:di + _TX_B_OFFSET]
	NextXTX:


	mov dx, word [cs:di + _Y_B_OFFSET]
	cmp dx, word [cs:di + _TY_B_OFFSET]
	jg YGreaterTY
	mov dx, word [cs:di + _TY_OFFSET]
	sub dx, word [cs:di + _Y_OFFSET]
	jmp NextYTY
	YGreaterTY:
	sub dx, word [cs:di + _TY_B_OFFSET]
	NextYTY:
	shl cx, 8
	shl dx, 8
	add cx, dx
	add cx, 8

	mov word[cs:(di + _JUMP_COUNT_B_OFFSET)], cx

	mov byte[cs:(di + _USED_B_OFFSET)], 1


	dec byte[cs:FootBallNum]
	jnz PutFB

	NoFootBall:

	ret

UpdateBoss:
	;Boss
	cmp byte[cs:BossHP], 0
	je BOSSDEAD
	;Boss Skill
	mov si, BOSS
	mov cx, word[cs: si + _X_OFFSET]
	mov dx, word[cs: si + _Y_OFFSET]
	add cx, 0x80
	add dx, 0x80
	shr cx, 8
	shr dx, 8
	%macro BossSkill 2
		push cx
		push dx
		add cx, %1
		add dx, %2
		TQ %1, %2
		pop dx
		pop cx
	%endmacro

	BossSkill 0,1
	BossSkill -1,0
	BossSkill 1,0
	BossSkill 0,-1

	BossSkill -1,-1
	BossSkill 1,-1
	BossSkill 1,1
	BossSkill -1,1

	push 0
	call AI
	mov si, BOSS
	call UpdatePlayer
	call DrawPlayer

	BOSSDEAD:
	ret

UpdatePower:
	mov si, Powers
	mov cx, TotalPower

	UpdatePowerIn:
	cmp byte[cs:si + _USED_P_OFFSET], 0
	je NextPower 
	dec word[cs:si + _COUNT_P_OFFSET]
	jnz PowerNotZero


	push dx
	push cx
	push bx
	push ax

	mov cx, word [cs:(si + _X_P_OFFSET)]
	mov dx, word [cs:(si + _Y_P_OFFSET)]

	shr cx, 8
	shr dx, 8
	mov ax, dx
	mov dx, WinCol
	mul dx
	add ax, cx
	mov bx, ax
	mov byte[cs:STATE_DATA + bx], 0

	pop ax
	pop bx
	pop cx
	pop dx
	
	mov byte[cs:si + _USED_P_OFFSET], 0


	PowerNotZero:
	
	call DrawPower	

	NextPower:	
	add si, PowerSize
	loop UpdatePowerIn 

	ret

WKCNINTTimer:

	cmp byte[cs:IsStartGame], 0
	je NOTSTARTGAME


	HAVESTARTGAME:
	push 0
	call Update

	call DrawMap
	call DrawHP

	call UpdateFootball

	;Bombs
	mov si, Bombs
	mov cx, TotalBomb

	UpdateBombs:
	cmp byte[cs:(si + _USED_B_OFFSET)], 0
	je NoUsedBomb
	call UpdateBomb

	cmp byte[cs:(si + _USED_B_OFFSET)], 0
	je NoUsedBomb

	call DrawBomb

	NoUsedBomb:

	add si, BombDataSize
	loop UpdateBombs

	;Roles
	cmp byte[cs:PlayerHP], 0
	je PlayerDEAD
	mov si, Players
	call KeyJudge
	call UpdatePlayer
	call DrawPlayer
	PlayerDEAD:

	call UpdateBoss

	;Power
	call UpdatePower

	;Info
	cmp byte[cs:PlayerHP], 0
	je LOSE
	cmp byte[cs:BossHP], 0
	je WIN
	jmp INFOEND

	WIN:
	mov word[cs:DrawRectW], 70 
	mov word[cs:DrawRectH], 35 
	mov word[cs:DrawSegment], WIN_SEG
	mov bx, 0
	mov cx, 30
	mov dx, 30 
	call DRAW
	jmp INFOPRESS

	LOSE:
	mov word[cs:DrawRectW], 70 
	mov word[cs:DrawRectH], 35 
	mov word[cs:DrawSegment], LOSE_SEG
	mov bx, 0
	mov cx, 30
	mov dx, 30 
	call DRAW
	jmp INFOPRESS

	INFOPRESS:
	mov byte[cs:IsStartGame], 0

	INFOEND:


	call UpdateScreen

	NOTSTARTGAME:

	mov al,20h
	out 20h,al
	out 0A0h,al
	iret



DrawCount dw 0
DrawXCount dw 0
DrawCol dw 0
DrawMapCol dw 0
DrawSegment dw 0x3000
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
	_GRAPH dw HUOYING_SEG
	_DIR dw 2
	_PAT dw 0
	_X	dw 700h
	_Y	dw 600h
	_TX	dw 700h
	_TY	dw 600h
	_ANI dw 0
	_V	dw 0x20
BOSS:
	_GRAPH2 dw BOSS_SEG
	_DIR2 dw 1
	_PAT2 dw 0
	_X2	dw 0b00h
	_Y2	dw 600h
	_TX2	dw 0b00h
	_TY2	dw 600h
	_ANI2 dw 0
	_V2	dw 0x08


%macro SetOffset_B 1
	%1_OFFSET equ (%1 - Bombs)
%endmacro

SetOffset_B _GRAPH_B
SetOffset_B _USED_B
SetOffset_B _POWER_B
SetOffset_B _COUNT_B
SetOffset_B _PAT_B
SetOffset_B _X_B
SetOffset_B _Y_B
SetOffset_B _TX_B
SetOffset_B _TY_B
SetOffset_B _ANI_B
SetOffset_B _JUMP_COUNT_B

Bombs:
	_GRAPH_B dw FOOTBALL_SEG
	_USED_B db 0
	_POWER_B db 3
	_COUNT_B dw 5 * UpdateTimes
	_PAT_B dw 0
	_X_B	dw 000h
	_Y_B	dw 000h
	_TX_B	dw 0a00h
	_TY_B	dw 0a00h
	_ANI_B dw 0
	_JUMP_COUNT_B dw 20
FirstBombEnd:

BombDataSize equ FirstBombEnd - Bombs
times BombDataSize * WinCol * WinRow db 0

%macro SetOffset_P 1
	%1_OFFSET equ (%1 - Powers)
%endmacro

SetOffset_P _USED_P
SetOffset_P _PAT_P
SetOffset_P _COUNT_P
SetOffset_P _X_P
SetOffset_P _Y_P

Powers:
	_USED_P db 0
	_PAT_P dw 0
	_COUNT_P dw 0 
	_X_P dw 0000h
	_Y_P dw 0000h
FirstPowersEnd:

PowerSize equ FirstPowersEnd - Powers
times PowerSize * WinCol * WinRow db 0

HPs:
	PlayerHP db 1
	BossHP db BossMaxHP
NOHARM:
	PlayerNOHARM dw 0
	BossNOHARM dw 0


MAPPIC0 db "MAPPIC  RES"
HUOYING db "HUOYING RES"
FOOTBALL db "FOOTBALLRES"
PEOPLE db "PEOPLE  RES"
BOSSName db "BOSS    RES"
PowerName db "POWER   RES"
WinName db "WIN     RES"
LoseName db "LOSE    RES"
TitleName db "TITLE   RES"

FootBallTC dw FootballTime * UpdateTimes
FootBallNum db 0
FootBallMaxNum db 3
FootBallIncCount dw FootballIncTime * UpdateTimes

IsStartGame db 0

MAP0:
%include "map0.asm"
MAP3:
%include "map3.asm"
PASSED_DATA:
%include "map3.asm" ; 非0的均不能走
STATE_DATA:
times WinCol * WinRow db 0
