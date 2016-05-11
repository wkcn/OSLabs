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

void memcpy(void *dest,void *src,int size){
	for (int i = 0;i < size;++i){
		*(((char*)dest)+i) = *(((char*)src)+i);
	}
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
	uint16_t res = hour * 3600 + minute * 60 + second;
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

extern "C" uint16_t GetKey();
extern "C" void memcpy(void *dest,const void *src,uint16_t n);

#define max(a,b) ((a)>(b)?(a):(b))
#define min(a,b) ((a)<(b)?(a):(b))

#endif
