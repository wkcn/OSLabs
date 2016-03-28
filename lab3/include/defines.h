#ifndef _DEFINES_H_
#define _DEFINES_H_

#include <stdint.h>

#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 25


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

typedef uint16_t osi; // default interger in OS


#endif
