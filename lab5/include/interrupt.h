#ifndef _INTERRUPT_H_
#define _INTERRUPT_H_
#include "defines.h"
#include "io.h"
//#include <stdint.h>

__attribute__((regparm(3)))
void WriteIVT(uint16_t id,uint16_t offset,uint16_t cs){
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
	uint16_t *int_ip = (uint16_t*)(id * 4);
	uint16_t *int_cs = (uint16_t*)(id * 4 + 2);
	*int_ip = (uint16_t)func;
	*int_cs = cs;
}

__attribute__((regparm(1)))
void ExecuteINT(uint16_t id){
	uint16_t ip, cs;
	asm volatile(
			"push es;"
			"mov es, ax;"
			"mov ax, es:[bx];"
			"mov bx, es:[bx + 2];"
			"pop es;"
			:"=a"(ip),"=b"(cs)
			:"a"(0),"b"(id * 4)
			);
	uint16_t ocs;
	asm volatile("mov ax, cs;":"=a"(ocs));
	//不知道为什么, 一定要加下面这一句, 否则崩溃:-(
	PrintHex(id);PrintStr(NEWLINE);
	char addr[4];
	*((uint16_t*)(addr)) = ip;
	*((uint16_t*)(addr+2)) = cs;
	asm volatile(
			"push es;"
			"mov es, ax;"
			"pushf;"
			"call far ptr es:[bx];"
			"pop es;"
			:
			:"a"(ocs),"b"(addr)
			);
}	

#endif
