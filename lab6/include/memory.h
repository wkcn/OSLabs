#ifndef _OSMEMORY_H_
#define _OSMEMORY_H_

#include <stdint.h>
#include "port.h"

const uint16_t BOX_SIZE = 512;
const uint16_t BOX_NUM = 512;
uint16_t SPACE_PROG_SEGMENT;
uint16_t SPACE_MEM_SEGMENT;
#define GET_PROG_SEGMENT asm volatile("int 0x21;":"=a"(SPACE_PROG_SEGMENT):"a"(0x0400))
#define GET_MEM_SEGMENT asm volatile("int 0x21;":"=a"(SPACE_MEM_SEGMENT):"a"(0x0A00))

__attribute__((regparm(1)))
bool BoxValid(uint16_t id){
	GET_MEM_SEGMENT;
	uint16_t bid = id / 8;
	unsigned char p = 0x80 >> (id % 8);
	char c;
	asm volatile(
			"push es;"
			"mov es, ax;"
			"mov ax, es:[bx];"
			"pop es;"
			:"=a"(c)
			:"a"(SPACE_MEM_SEGMENT),"b"(bid)
			);
	return !(c & p);
}


__attribute__((regparm(1)))
void UseBox(uint16_t id){
	GET_MEM_SEGMENT;
	uint16_t bid = id / 8;
	unsigned char p = 0x80 >> (id % 8);
	PrintChar('\r');
	char c;
	asm volatile(
			"push es;"
			"mov es, ax;"
			"mov ax, es:[bx];"
			"pop es;"
			:"=a"(c)
			:"a"(SPACE_MEM_SEGMENT),"b"(bid)
			);
	c |= p;
	asm volatile(
			"push es;"
			"mov es, ax;"
			"mov es:[bx],cx;"
			"pop es;"
			:
			:"a"(SPACE_MEM_SEGMENT),"b"(bid),"c"(c)
			);
}

__attribute__((regparm(1)))
void ReleaseBox(uint16_t id){
	GET_MEM_SEGMENT;
	uint16_t bid = id / 8;
	unsigned char p = 0x80 >> (id % 8);
	char c;
	asm volatile(
			"push es;"
			"mov es, ax;"
			"mov ax, es:[bx];"
			"pop es;"
			:"=a"(c)
			:"a"(SPACE_MEM_SEGMENT),"b"(bid)
			);

	c &= ~p;

	asm volatile(
			"push es;"
			"mov es, ax;"
			"mov es:[bx],cx;"
			"pop es;"
			:
			:"a"(SPACE_MEM_SEGMENT),"b"(bid),"c"(c)
			);
}

//返回段地址!
__attribute__((regparm(1)))
uint16_t AllocateMEM(uint16_t need){

	//先用旧版本
	GET_PROG_SEGMENT;
	uint16_t PROG_SEG_S;
	ReadPort(3,&PROG_SEG_S,sizeof(PROG_SEG_S));
	uint16_t addrseg = SPACE_PROG_SEGMENT + PROG_SEG_S;
	PROG_SEG_S += need >> 4;
	PrintNum(PROG_SEG_S);
	PrintStr(NEWLINE);
	WritePort(3,&PROG_SEG_S,sizeof(PROG_SEG_S));
	return addrseg;

	uint16_t z = (need + BOX_SIZE - 1) / BOX_SIZE;// 需要的盒子数~
	uint16_t i = 0;
	bool success = false;
	while(true){
		for (;i < BOX_NUM*8 && !BoxValid(i);++i);// 找到可用的位置
		uint16_t count = 1;
		for (;i + count < BOX_NUM * 8 && (count < z) && BoxValid(i+count);++count);
		if (count >= z){
			success = true;
			break;
		}else{
			//暂时不考虑i + count >= BOX_NUM * 8
			if (i + count >= BOX_NUM * 8){
				break;
			}else{
				//不可用
				i = i + count;
			}
		}
	}
	for (uint16_t w = i;w < i+z;++w){
		UseBox(w);
	}
	GET_PROG_SEGMENT;
	return SPACE_PROG_SEGMENT + i * (512>>4);
}	

__attribute__((regparm(2)))
void ReleaseMEM(uint16_t seg, uint16_t size){
	uint16_t z = (size + BOX_SIZE - 1) / BOX_SIZE;// 需要的盒子数~
	GET_PROG_SEGMENT;
	uint16_t i = (seg - SPACE_PROG_SEGMENT) / (512>>4);
	for (uint16_t w = i;w < i+z;++w){
		ReleaseBox(w);
 	}
} 
#endif
