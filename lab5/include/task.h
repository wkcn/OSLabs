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
	EMPTY, RUNNING, SUSPEND
};

Processes _p;

__attribute__((regparm(2)))
void SetTaskState(uint16_t id, uint16_t state){
	uint16_t offset = id * PCBSize + ((char*)&_p.FLAGS - (char*)&_p);
	asm volatile(
			"push es;"
			"mov es, dx;"
			"mov es:[bx], ax;"
			"pop es;"
			:
			:"a"(state),"b"(offset),"d"(PCB_SEGMENT)
			);
}

void KillAll(){
	for (int i = 1;i < MaxRunNum;++i){
		SetTaskState(i,EMPTY);
	}
}

__attribute__((regparm(1)))
void SetTaskAll(uint16_t state){
	for (int i = 1;i < MaxRunNum;++i){
		SetTaskState(i,state);
	}
}

#endif
