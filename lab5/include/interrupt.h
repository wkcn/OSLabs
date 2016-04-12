#ifndef _INTERRUPT_H_
#define _INTERRUPT_H_
#include "defines.h"
#include "io.h"

__attribute__((regparm(3)))
void WriteIVT(uint16_t id,uint16_t offset,uint16_t cs){
	PrintNum(offset);
	PrintStr(NEWLINE);
	PrintNum(cs);
	PrintStr(NEWLINE);
	asm volatile(
			"push si;push es;"
			"mov es, ax;"
			"mov si, bx;"
			"mov es:[si], cx;"
			"mov es:[si+2], dx;"
			"pop es;pop si;"
			:
			:"a"(0),"b"(id * 4),"c"(offset),"d"(cs)
			);
}

__attribute__((regparm(2)))
void WriteIVT(uint16_t id, void (*func)()){
	uint16_t cs;
	asm volatile("mov ax, cs;":"=a"(cs));
	WriteIVT(id,(long)func,cs);
}

void KeyBoardINT(){
	static bool pushed = true;
	if (pushed){
		DrawText("OUCH! OUCH!",0,24,LCARM);
	}else{
		DrawText("           ",0,24,LCARM);
	}
	pushed = !pushed;
	asm volatile(
			"in al,0x61;"
			"mov al,0x20;"
			"out 0x20,al;"
			:
			:
			:"ax"
			);
}


#endif
