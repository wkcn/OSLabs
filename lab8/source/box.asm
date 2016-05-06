;;注意:由于使用了分时系统, 这里取消时钟中断!
;;**********************************************************************************
;;*   本程序利用时钟中断,在文本方式显示终端上,沿四边用大字字母反复画框.
;;*   程序设计:凌应标
;;*   最新日期:2013/3/27
;;*
;;*
;;*
;;**********************************************************************************

        org 100h					; 程序加载到100h，可用于生成COM

; 设置时钟中断向量（08h），初始化段寄存器
	mov ax,cs 
	mov ds,ax					; DS = CS

	;使用时钟决定颜色
	mov ax, 0x0200
	int 0x1a
	cmp dh, 0
	je NC
	and dh, 0Fh
	mov byte[color], dh
	NC:

	mov ax, 3
	int 10h
	jmp Timer


; 时钟中断处理程序
	x dw 0                      ; 当前字符显示位置的行号,0~24
	y dw 79                     ; 当前字符显示位置的列号,0~79
	xmin dw 0
	xmax dw 24
	ymin dw 0
	ymax dw 79
	rdlu db 3                   ; 当前画框的方向, 1-向右,2-向下,3-向左,4-向上
	char db 'A'                 ; 当前显示字符
	count dw 0
	color db 07h
	
Timer:
	
left:
    mov al,byte[rdlu]           ;向左 
	cmp al,3
	jnz down
	mov ax,word[y]               ;最左一列?
	cmp ax, word [ymin]
	jz l2d
	dec byte[y]
	jmp show

right:
    mov al,byte[rdlu]           ;左-->右 
	cmp al,1
	jnz up
	mov ax,word[y]               ;最后一列?
	cmp ax, word [ymax]
	jz r2u
	inc byte[y]
	jmp show
	
down:
    mov al,byte[rdlu]           ;向下 
	cmp al,2
	jnz right
	mov ax,word[x]               ;最后一行?
	cmp ax, word [xmax]
	jz d2r
	inc byte[x]
	jmp show

	
up:
    mov al,byte[rdlu]           ;向上 
	cmp al,4
	jnz left
	mov ax,word[x]               ;最上一行?
	cmp ax, word [xmin]
	jz u2l
	dec byte[x]
	jmp show

ChangeColor:
	push ax
	mov al,byte[color]
	inc al
	and al, 0Fh
	cmp al, 0
	jne ChangeColorEnd
	mov al,1
	ChangeColorEnd:
	mov byte[color],al
	pop ax
	ret

u2l:
    mov byte[rdlu],3           ;改为向左
	dec byte[y]
	dec word[ymax]
	call ChangeColor

	mov al,byte[char]
	cmp al,'Z'
	jz returntoa
	inc byte[char]
	
	jmp show
r2u:
    mov byte[rdlu],4           ;改为向 上
	dec byte[x]
	dec byte[xmax]
	call ChangeColor

	jmp show
l2d:
    mov byte[rdlu],2           ;改为向下
	inc byte[x]
	inc byte[xmin]
	call ChangeColor

	jmp show
d2r:
    mov byte[rdlu],1           ;改为向右
	inc byte[y]
	inc byte[ymin]
	call ChangeColor

	jmp show
	
returntoa:
    mov byte[char],'A' 
	jmp show

show:	
    xor ax,ax                      ; 计算当前字符的显存地址 gs:((80*x+y)*2)
    mov ax,word[x]
	mov bx,80                  ; (80*x
	mul bx
	add ax,word[y]             ; (80*x+y)
	mov bx,2
	mul bx                     ; ((80*x+y)*2)
	mov bp,ax

	mov ah,byte[color]

	mov al,byte[char]	   ; AL = 显示字符值（默认值为20h=空格符）
	mov bx, 0xB800
	mov es, bx
	mov bx, word[es:bp]
	cmp bl, 0
	mov word[es:bp],ax  	   ;   显示字符的ASCII码值

	inc word[count]
	cmp word[count], 80*25
	jle end
	mov ax,3
	int 10h
	int 20h ; exit

end:
	mov cx, 0xFFFF
	DELAY:
		mov ax, 100
		DELAY_IN:
			dec ax
			cmp ax, 0
		jne DELAY_IN
	loop DELAY
	jmp Timer 
	
	

	
;********************************************************************************************************
;*  修改一: 本程序画框是顺时针的,请把方向改为逆时间
;*  修改二: 本程序画框重复画最大的框,请改为先画最大的框,之后在所画的框内画最大的框,如此反复,直到画满屏幕
;*
;********************************************************************************************************
