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

uint16_t getkey(){
	uint16_t key;
	asm volatile(
			"mov ah, 0x01;"
			"int 0x16;"
			"jz KEYEND;"
			"mov ah, 0;"
			"int 0x16;"
			"jmp KEYEND;"
			"mov ax, 0;"
			"KEYEND:;"
			:"=a"(key)
			);
	return key;
}

uint16_t GetCursor(){
	// H: row
	// L  column
	uint16_t p;
	asm volatile("int 0x10;"
			:"=d"(p)
			:"a"(0x0300),"b"(0)
			);
	return p;
}

__attribute__((regparm(2)))
void SetCursor(uint16_t r, uint16_t c){
	asm volatile("int 0x10;"
			:
			:"a"(0x0200),"b"(0),"d"((r << 8) | c)
			);
}

void cls(){
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




__attribute__((regparm(3)))
void DrawChar(char ch,uint16_t r,uint16_t c,uint16_t color = 0x07){
	uint16_t k = (r * 80 + c) * 2;
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

__attribute__((regparm(3)))
void DrawText(const char *str,uint16_t r,uint16_t c,uint16_t color = 0x07){
	while(*str){
		DrawChar(*(str++),r,c++,color);
	}
}

__attribute__((regparm(2)))
void PrintChar(char ch, uint16_t color = 0x07){
	//Use 10h interupt to get right cursor position
	uint16_t ocp = GetCursor();
	uint16_t orow = ocp >> 8;
	uint16_t ocol = ocp & 0x00FF;
	asm volatile("int 0x10;"
                :
                : "a"(0x0E00 | ch), "b"(color)
				);
	//color
	if (ch != '\n' && ch != '\b' && ch != '\r'){
		uint16_t cp = GetCursor();
		uint16_t row = cp >> 8;
		uint16_t col = cp & 0x00FF;
		DrawChar(ch,orow,ocol,color);
		SetCursor(row,col);
	}
}

__attribute__((regparm(2)))
void PrintStr(const char *str, uint16_t color = 0x07){
	while(*str){
		PrintChar(*str,color);
		++str;
	}
}

__attribute__((regparm(3)))
void PrintStrN(const char *str, uint16_t len, uint16_t color = 0x07){
	for(uint16_t i = 0;i < len;++i){
		PrintChar(*str,color);
		++str;
	}
}

__attribute__((regparm(1)))
int PrintNum(int num, uint16_t color = WHITE){
	char temp[16];
	if (num < 0){
		PrintChar('-');
		num = -num;
	}
	int i = 0;
	do{
		temp[i] = num % 10;
		num /= 10;
		++i;
	}while(num > 0);

	for (int j = i - 1;j >= 0;--j){
		PrintChar(temp[j] + '0', color);
	}
	return i;
}

const char ch[16] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
__attribute__((regparm(1)))
void PrintHex(char num, uint16_t color = WHITE){
	PrintChar(ch[(num>>4)&0xF],color);
	PrintChar(ch[(num)&0xF],color);
}
__attribute__((regparm(1)))
void PrintHex2(uint16_t num, uint16_t color = WHITE){
	PrintHex(char((num >> 8) & 0xFF),color);
	PrintHex(char((num) & 0xFF),color);
}
//class ostream{
//public:
//	osi color;
//};
struct ostream{
	uint16_t color;
};

#define endl "\n\r"
__attribute__((regparm(2)))
ostream& operator<<(ostream &os,int num){
	PrintNum(num,os.color);
	return os;
}

__attribute__((regparm(2)))
ostream& operator<<(ostream &os,const char *str){
	PrintStr(str,os.color);
	return os;
}

#endif
