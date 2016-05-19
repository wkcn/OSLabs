#ifndef _DEFINES_H_
#define _DEFINES_H_

#include <stdint.h>

#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 25

uint16_t PCB_SEGMENT;
uint16_t PROG_SEGMENT;
uint16_t MSG_SEGMENT;
uint16_t MaxRunNum;

void INIT_SEGMENT(){ 
	asm volatile("int 0x21;"
			:"=a"(PCB_SEGMENT)
			:"a"(0x0300)
			);

	asm volatile("int 0x21;"
			:"=a"(PROG_SEGMENT)
			:"a"(0x0400)
			);

	asm volatile("int 0x21;"
			:"=a"(MSG_SEGMENT)
			:"a"(0x0500)
			);
	asm volatile("int 0x21;"
			:"=a"(MaxRunNum)
			:"a"(0x0600)
			);
} 

const char *NEWLINE = "\r\n";

#define STREAM_MAX_LEN 16
struct stream{
	char str[STREAM_MAX_LEN];
	uint16_t len;
	void put(char ch){
		if (len+1 < STREAM_MAX_LEN){
			str[len++] = ch;
			str[len] = 0;
		}
	}
	void pop(){
		if (len > 0){
			str[--len] = 0;
		}
	}
	stream(){
		len = 0;
	}
};

//Font Color
enum Color{
	BLACK = 0x00,
	BLUE,
	GREEN,
	CYAN, // 青色
	RED,
	CARM, // 洋红
	BROWN,
	WHITE,
	GRAY,
	LBLUE,
	LGREEN,
	LCYAN,
	LRED,
	LCARM,
	YELLOW,
	LWHITE
};

struct Addr{
	uint16_t segment;
	uint16_t offset;
};

__attribute__((regparm(3)))
void memcpy(void *dest,const void *src,uint16_t size){
	for (uint16_t i = 0;i < size;++i){
		*(((char*)dest)+i) = *(((char*)src)+i);
	}
}

__attribute__((regparm(3)))
void memcpy(Addr dest,const Addr src,uint16_t size){
	asm volatile(
			"push ds;push si;push es;push di;"
			);
	asm volatile(
			"mov si, bx;mov es, cx;mov di,dx;"
			:
			:"b"(src.offset),"c"(dest.segment),"d"(dest.offset)
			);
	asm volatile(
			"mov ds, ax;"
			"MEMCPYAA:movsb;loop MEMCPYAA;"
			:
			:"a"(src.segment),"c"(size)
			);
	asm volatile(
			"pop di;pop es;pop si;pop ds;"
			);
}

__attribute__((regparm(1)))
Addr GetAddr(void *d){
	Addr a;
	asm volatile("mov ax, cs;":"=a"(a.segment));
	a.offset = (unsigned long)d;
	return a;
}

__attribute__((regparm(2)))
Addr MakeAddr(uint16_t cs, uint16_t ip){
	Addr a;
	a.segment = cs;
	a.offset = ip;
	return a;
}

__attribute__((regparm(1)))
uint16_t BCD2HEX(uint16_t bcd){
	return ((bcd >> 12) & 0xF) * 1000 + \
					 ((bcd >> 8) & 0xF) * 100 + \
					 ((bcd >> 4) & 0xF) * 10 + \
					 ((bcd) & 0xF);
}

uint16_t clock(){
	uint16_t chcl, dhdl;
	uint16_t hour,minute,second;
	asm volatile(
			"int 0x1a;"
			:"=c"(chcl),"=d"(dhdl)
			:"a"(0x0200)
			);
	hour = (chcl >> 8) & 0xFF;
	minute = (chcl) & 0xFF;
	second = (dhdl >> 8) & 0xFF;
	uint16_t res = BCD2HEX(hour) * 3600 + BCD2HEX(minute) * 60 + BCD2HEX(second);
	return res;
}

__attribute__((regparm(1)))
	void sleep(uint16_t seconds){
		uint16_t old = clock();
		uint16_t j = clock() - old;
		//10 seconds 是为了避免Bug
		while(j <= seconds || j >= 10)j = clock() - old;
	}

typedef char db;
typedef uint16_t dw;
typedef uint32_t dd;
typedef uint64_t dq;

#define max(a,b) ((a)>(b)?(a):(b))
#define min(a,b) ((a)<(b)?(a):(b))

#endif
