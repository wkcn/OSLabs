#ifndef _MEMORY_H_
#define _MEMORY_H_

#include "io.h"

__attribute__((regparm(1)))
int allocate(int needSize){
	int addr;
	asm volatile(
			"int 0x23;"
			:"=a"(addr)
			:"a"(0x0100),"c"(needSize)
			);
	return addr;
}

__attribute__((regparm(2)))
void free(int addr, int freeSize){
	asm volatile(
			"int 0x23;"
			:
			:"a"(0x0000),"b"(addr),"c"(freeSize)
			);
}

#endif
