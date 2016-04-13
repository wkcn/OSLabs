#ifndef _TASK_H_
#define _TASK_H_

#include <stdint.h>

const uint16_t PCB_SEGMENT = 0x3000;
const uint16_t MaxRunNum = 16;

typedef char db;
typedef uint16_t dw;
typedef uint32_t dd;
typedef uint64_t dq;

#pragma pack (1)
struct Processes{
	db ID;
	db STATE;
	db NAME[16];
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
	T_EMPTY, T_RUNNING, T_SUSPEND, T_TEMP, T_DEAD = 4
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

void KillAll(){
	for (int i = 1;i < MaxRunNum;++i){
		SetTaskState(i,T_DEAD);
	}
}

__attribute__((regparm(2)))
void SetAllTask(uint8_t toState,uint8_t fromState){
	for (int i = 1;i < MaxRunNum;++i){
		if (GetTaskState(i) == fromState){
			SetTaskState(i, toState);
		}
	}
}

#endif
