#ifndef _SCREEN_H_
#define _SCREEN_H_

#include <stdint.h>

const uint16_t SCREEN_S = (80 * 25 * 2 / 16);

__attribute__((regparm(1)))
void SaveScreen(uint16_t id){
	uint16_t SCREEN_SEGMENT;
	asm volatile("sti;int 0x21;":"=a"(SCREEN_SEGMENT):"a"(0x0B00));
	asm volatile("push ds;push es;mov es, ax;mov ds, dx"::"a"(0xB800),"d"(SCREEN_SEGMENT + SCREEN_S * (id - 1)));
	for (int i = 0;i < 80 * 25 * 2;++i){
		asm volatile("mov al, es:[bx];mov ds:[bx],al;"::"b"(i):"ax");
	}
	asm volatile("pop es;pop ds;");
}

__attribute__((regparm(1)))
void LoadScreen(uint16_t id){
	uint16_t SCREEN_SEGMENT;
	asm volatile("sti;int 0x21;":"=a"(SCREEN_SEGMENT):"a"(0x0B00));
	asm volatile("push ds;push es;mov es, ax;mov ds, dx"::"a"(0xB800),"d"(SCREEN_SEGMENT + SCREEN_S * (id - 1)));
	for (int i = 0;i < 80 * 25 * 2;++i){
		asm volatile("mov al, ds:[bx];mov es:[bx],al;"::"b"(i):"ax");
	}
	asm volatile("pop es;pop ds;");
}

#endif
