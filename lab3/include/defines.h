#ifndef _DEFINES_H_
#define _DEFINES_H_

#include <stdint.h>

#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 25

const char *NEWLINE = "\r\n";

struct stream{
	char str[128];
	int len;
	void put(char ch){
		str[len++] = ch;
		str[len] = 0;
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

osi strlen(char *s){
	osi i = 0;
	while(*s)i++;
	return i;
}

#endif
