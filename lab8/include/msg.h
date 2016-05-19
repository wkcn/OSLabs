#ifndef _MSG_H_
#define _MSG_H_

#include "msg_base.h"

__attribute__((regparm(2)))
bool Send(uint8_t target, MsgPack *p){
	uint16_t retn;
	asm volatile(
			"mov bx, cs;"
			"int 0x25;"
			:"=a"(retn)
			:"a"(0x3000 | target), "c"((unsigned long)p)
			:"bx"
			);
	return retn;
}	

__attribute__((regparm(1)))
bool IRecvAll(MsgPack *p){
	uint16_t retn;
	asm volatile(
			"mov bx, cs;"
			"int 0x25;"
			:"=a"(retn)
			:"a"(0x4300), "c"((unsigned long)p)
			:"bx"
			);
	return retn;
}
#endif
