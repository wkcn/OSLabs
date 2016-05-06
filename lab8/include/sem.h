#ifndef _SEM_H_
#define _SEM_H_
#include <stdint.h>

__attribute__((regparm(1)))
void semWait(uint8_t sid){
	uint16_t ax = 0x0100 | sid;
	/*
	asm volatile(
			"pushf;"
			"call far ptr cs:[bx];"
			:
			:"a"(ax),"b"(0x25 * 4)
			);
			*/
	asm volatile("int 0x25"::"a"(ax));
}

__attribute__((regparm(1)))
void semSignal(uint8_t sid){
	uint16_t ax = 0x0200 | sid;
	/*
	asm volatile(
			"pushf;"
			"call far ptr cs:[bx];"
			:
			:"a"(ax),"b"(0x25 * 4)
			);
			*/
	
	asm volatile("int 0x25"::"a"(ax));
}


__attribute__((regparm(1)))
uint8_t semCreate(int8_t count){
	uint16_t ax = 0x0000 | (uint16_t)count;
	uint8_t sid;

	/*
	asm volatile(
			"pushf;"
			"call far ptr cs:[bx];"
			:"=a"(sid)
			:"a"(ax),"b"(0x25 * 4)
			);
			*/
	
	asm volatile("int 0x25":"=a"(sid):"a"(ax));
	return sid;
}

__attribute__((regparm(1)))
void semDel(uint8_t sid){
	uint16_t ax = 0x0400 | sid;
	asm volatile("int 0x25"::"a"(ax));
}

#endif
