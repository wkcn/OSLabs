#ifndef _DEFINES_H_
#define _DEFINES_H_

#include <stdint.h>

#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 25

typedef uint16_t osi; // default interger in OS

osi strlen(char *s){
	osi i = 0;
	while(*s)i++;
	return i;
}

#endif
