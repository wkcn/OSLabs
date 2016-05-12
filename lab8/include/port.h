#ifndef _PORT_H_
#define _PORT_H_
/*
	;22H进程, 进程通信

	;ah = 00h 读
	;ah = 01h 写
	;ah = 02h 信号量设置(bh=0, 清零; bh=1, 加1; bh=2, 减1; bh=3, 设置为bl值)
	;ah = 03h 设置端口
	;ah = 04h 关闭端口
	;ah = 05h 只返回信号量
	;al = 端口值
	;基地址bx, 缓存大小cx, 段地址dx 
	;返回信号量(ax)
*/
#include "defines.h"
#include "sem.h"
#include "portsList.h"

__attribute__((regparm(3)))
void SetPort(uint8_t portID, void* varAddr, uint16_t size){
	uint16_t cs;
	asm volatile("mov ax,cs;":"=a"(cs));
	asm volatile("int 0x22;"
			:
			:"a"((3 << 8) | portID), "b"(varAddr), "c"(size), "d"(cs)
			);
}

__attribute__((regparm(3)))
uint8_t ReadPort(uint8_t portID, void* varAddr, uint16_t size){
	uint16_t cs;
	uint8_t v;
	asm volatile("mov ax,cs;":"=a"(cs));
	asm volatile("int 0x22;"
			:"=a"(v)
			:"a"((0 << 8) | portID), "b"(varAddr), "c"(size), "d"(cs)
			);
	return v;
}

__attribute__((regparm(3)))
void WritePort(uint8_t portID, void* varAddr, uint16_t size){
	uint16_t cs;
	asm volatile("mov ax,cs;":"=a"(cs));
	asm volatile("int 0x22;"
			:
			:"a"((1 << 8) | portID), "b"(varAddr), "c"(size), "d"(cs)
			);
}


__attribute__((regparm(2)))
void SetPortMsgV(uint8_t portID, uint8_t v){
	uint16_t cs;
	asm volatile("mov ax,cs;":"=a"(cs));
	asm volatile("int 0x22;"
			:
			:"a"((2<<8) | portID), "b"((3<<8) | v)
			);
} 


__attribute__((regparm(1)))
uint8_t GetPortMsgV(uint8_t portID){
	uint8_t v;
	asm volatile("int 0x22;"
			:"=a"(v)
			:"a"((5<<8) | portID)
			);
	return v;
}

/*端口信号量支持*/

__attribute__((regparm(2)))
void PortSemCreate(uint8_t portID, int8_t count){
	SetPortMsgV(portID, semCreate(count));
}

__attribute__((regparm(1)))
void PortSemDel(uint8_t portID){
	uint8_t sid = GetPortMsgV(portID);
	semDel(sid);
}


__attribute__((regparm(1)))
void PortSemWait(uint8_t portID){
	uint8_t sid = GetPortMsgV(portID);
	semWait(sid);
}

__attribute__((regparm(1)))
void PortSemSignal(uint8_t portID){
	uint8_t sid = GetPortMsgV(portID);
	semSignal(sid);
}
#endif
