#ifndef _IO_H_
#define _IO_H_

#include "defines.h"

char getchar(){
	// 获得一个按键（需等待）
	char ch;
	asm volatile("int 0x16;"
			:"=a"(ch)
			:"a"(0x1000)
			);
	return ch;
}

osi GetCursor(){
	// H: row
	// L  column
	osi p;
	asm volatile("int 0x10;"
			:"=d"(p)
			:"a"(0x0300),"b"(0)
			);
	return p;
}

void SetCursor(osi r, osi c){
	asm volatile("int 0x10;"
			:
			:"a"(0x0200),"b"(0),"d"((r << 8) | c)
			);
}

void CLS(){
 	/*
	AH = 06h to scroll up
       = 07h to scroll down
    AL = Number of lines to scroll (if zero, entire window is blanked)
    BH = Attribute to be used for blanked area
    CH = y coordinate, upper left corner of window
    CL = x coordinate, upper left corner of window
    DH = y coordinate, lower right corner of window
    DL = x coordinate, lower right corner of window 
	*/
	asm volatile("int 0x10"
		:
		:"a"(3)
		);
}  




void DrawChar(char ch,osi r,osi c,osi color = 0x07){
	osi k = (r * 80 + c) * 2;
	asm volatile(
				"push es;"
				"mov es, ax;"
				"mov es:[bx],cx;"
				"pop es;"
				:
				:"a"(0xB800),"b"(k), "c"((color<<8) | ch)
                :
			);
}

void DrawText(const char *str,osi r,osi c,osi color = 0x07){
	while(*str){
		DrawChar(*(str++),r,c++,color);
	}
}

void PrintChar(char ch, osi color = 0x07){
	//Use 10h interupt to get right cursor position
	osi ocp = GetCursor();
	osi orow = ocp >> 8;
	osi ocol = ocp & 0x00FF;
	asm volatile("int 0x10;"
                :
                : "a"(0x0E00 | ch), "b"(color)
				);
	//color
	if (ch != '\n' && ch != '\b' && ch != '\r'){
		osi cp = GetCursor();
		osi row = cp >> 8;
		osi col = cp & 0x00FF;
		DrawChar(ch,orow,ocol,color);
		SetCursor(row,col);
	}
}

void PrintStr(const char *str, osi color = 0x07){
	while(*str){
		PrintChar(*str,color);
		++str;
	}
}

//Read FLOPPY
void ReadDISK(osi secAddr, osi offset, osi driNum, osi headNum, osi cyl, osi start, osi len){
	/*
	asm volatile(
			"push es;"
			"mov es, ax;"
			"mov ah,2;"
			"mov al,%0;"
			"pop es;"
			:
			:"r"(len),"a"(secAddr),"b"(offset),"c"((cyl << 8)|start),"d"((headNum << 8) | driNum)
			);
	*/
}

void ReadProgram(osi id){
	//ReadDisk(0x8000,0x100,);
}

#endif
