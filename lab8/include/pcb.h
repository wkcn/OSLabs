#ifndef _PCB_H_
#define _PCB_H_

#include "defines.h"
#include "io.h"

typedef char db;
typedef uint16_t dw;
typedef uint32_t dd;
typedef uint64_t dq;

#pragma pack (1)
struct Processes{
	db ID;
	db STATE;
	db NAME[16];
	db KIND; // 0: 普通进程, 1: forked进程, 2: 线程
	db PARENT_ID;
	db BLOCK_NEXT;
	db PRIORITY;
	dw SEG;
	dw SSIZE;
	dw SIZE;
	dw ES;
	dw DS;
	dw DI;
	dw SI;
	dw BP;
	dw SP;
	dw BX;
	dw DX;
	dw CX;
	dw AX;
	dw SS;
	dw IP;
	dw CS;
	dw FLAGS;
};

const uint16_t PCBSize = sizeof(struct Processes);

enum TASK_STATE{
	T_EMPTY, T_RUNNING, T_SUSPEND, T_READY, T_DEAD = 4, T_BLOCKED
};

enum TASK_KIND{
	K_PROG, K_FORK, K_THREAD
};

Processes _p;

__attribute__((regparm(2)))
void SetTaskState(uint8_t id, uint8_t state){
	uint16_t offset = id * PCBSize + ((char*)&_p.STATE - (char*)&_p);
	asm volatile(
			"push es;"
			"mov es, dx;"
			"mov es:[bx], al;"
			"pop es;"
			:
			:"a"(state),"b"(offset),"d"(PCB_SEGMENT)
			);
}

__attribute__((regparm(1)))
uint8_t GetTaskState(uint8_t id){
	uint16_t offset = id * PCBSize + ((char*)&_p.STATE - (char*)&_p);
	uint8_t state;
	asm volatile(
			"push es;"
			"mov es, dx;"
			"mov dl, es:[bx];"
			"pop es;"
			:"=d"(state)
			:"b"(offset),"d"(PCB_SEGMENT)
			);
	return state;
}

__attribute__((regparm(3)))
void SetTaskAttr(uint8_t id, void *something, uint8_t value){
	uint16_t offset = id * PCBSize + ((char*)something - (char*)&_p);
	asm volatile(
			"push es;"
			"mov es, dx;"
			"mov es:[bx], al;"
			"pop es;"
			:
			:"a"(value),"b"(offset),"d"(PCB_SEGMENT)
			);
}

__attribute__((regparm(3)))
void SetTaskAttr(uint8_t id, void *something, uint16_t value){
	uint16_t offset = id * PCBSize + ((char*)something - (char*)&_p);
	asm volatile(
			"push es;"
			"mov es, dx;"
			"mov es:[bx], ax;"
			"pop es;"
			:
			:"a"(value),"b"(offset),"d"(PCB_SEGMENT)
			);
}

__attribute__((regparm(3)))
void GetTaskAttr(uint8_t id, void *something, uint8_t &value){
	uint16_t offset = id * PCBSize + ((char*)something - (char*)&_p);
	asm volatile(
			"push es;"
			"mov es, dx;"
			"mov al, es:[bx];"
			"pop es;"
			:"=a"(value)
			:"b"(offset),"d"(PCB_SEGMENT)
			);
}

__attribute__((regparm(3)))
void GetTaskAttr(uint8_t id, void *something, uint16_t &value){

	uint16_t offset = id * PCBSize + ((char*)something - (char*)&_p);
	asm volatile(
			"push es;"
			"mov es, dx;"
			"mov ax, es:[bx];"
			"pop es;"
			:"=a"(value)
			:"b"(offset),"d"(PCB_SEGMENT)
			);
}


__attribute__((regparm(2)))
void SetAllTask(uint8_t toState,uint8_t fromState){
	for (int i = 1;i < MaxRunNum;++i){
		if (GetTaskState(i) == fromState){
			SetTaskState(i, toState);
		}
	}
}

uint8_t FindEmptyPCB(){
	for (int i = 1;i < MaxRunNum;++i){
		if (GetTaskState(i) == T_EMPTY)return i;
	}
	return 0;
}

__attribute__((regparm(1)))
void LoadPCB(uint8_t id){
	uint16_t PCBOffset = PCBSize * id;
	char ch;
	asm volatile("push es;mov es, ax;"::"a"(PCB_SEGMENT));
	for (int i = 0;i < PCBSize;++i){
		asm volatile(
				"mov al, es:[bx];"
				:"=a"(ch)
				:"b"(PCBOffset + i)
				);
		*(((char*)&_p) + i) = ch;
	}
	asm volatile("pop es;");
}

__attribute__((regparm(1)))
void WritePCB(uint8_t id){
	uint16_t PCBOffset = PCBSize * id;
	asm volatile("push es;mov es, ax;"::"a"(PCB_SEGMENT));
	for (int i = 0;i < PCBSize;++i){
		asm volatile(
				"mov es:[bx], al;"
				:
				:"a"(*(((char*)&_p) + i)),"b"(PCBOffset + i)
				);
	}
	asm volatile("pop es;");
}

__attribute__((regparm(1)))
void KillTask(uint8_t id){
	if (id != 0 && GetTaskState(id) != T_EMPTY){
		SetTaskState(id, T_DEAD);
		//PrintStr("Kill Task Success!\r\n");
	}else{
		//PrintNum(id);
		//PrintStr("Kill Task Fail!\r\n",RED);
	}
}

__attribute__((regparm(3)))
void SetTaskState(uint8_t id, uint8_t toState, uint8_t fromState){
	if (!id)return;
	if (GetTaskState(id) == fromState)SetTaskState(id, toState);
}

uint16_t GetRunID(){
	uint16_t runid;
	asm volatile("int 0x21;":"=a"(runid):"a"(0x0200));
	return runid;
}

#define ScheduleOFF	asm volatile("sti;int 0x21;"::"a"(0x0700))

#define Schedule asm volatile("sti;int 0x08;")

#define ScheduleON asm volatile("sti;int 0x21;"::"a"(0x0800))

#define INC_RunNum asm volatile("sti;int 0x21;"::"a"(0x0900))

#endif
