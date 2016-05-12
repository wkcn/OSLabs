#ifndef _PROG_H_
#define _PROG_H_

#include <stdint.h>

struct ReadyProg{
	uint8_t uid;
	char filename[12];
	uint16_t allocatedSize;
};

#endif
