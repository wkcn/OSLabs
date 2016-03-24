#ifndef _IO_H_
#define _IO_H_

#include "defines.h"

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
	asm volatile("int 0x0010"
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

void DrawText(char *str,osi r,osi c,osi color = 0x07){
	while(*str){
		DrawChar(*str,r,c++);
		str++;
	}
}

#endif
