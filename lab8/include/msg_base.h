#ifndef _MSG_BASE_H_
#define _MSG_BASE_H_

#include "defines.h"

struct MsgPack{
	uint16_t func;
	Addr data; // 指向数据
	uint16_t size;
};

struct Message{
	bool used;
	bool visited;
	uint16_t id; 
	uint8_t source, target;
	uint16_t next;
	Addr pack; // 指向MsgPack
};
#endif
