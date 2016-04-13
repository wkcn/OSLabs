#ifndef _TASK_H_
#define _TASK_H_

#include <stdint.h>
#include "io.h"

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
	T_EMPTY, T_RUNNING, T_SUSPEND, T_READY, T_DEAD = 4
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
	for (uint8_t i = 1;i < MaxRunNum;++i){
		uint8_t state = GetTaskState(i);
		if (state != T_EMPTY){
			SetTaskState(i, T_DEAD);
		}
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
		PrintStr("Kill Task Success!\r\n");
	}else{
		//PrintNum(id);
		PrintStr("Kill Task Fail!\r\n",RED);
	}
}

__attribute__((regparm(3)))
void SetTaskState(uint8_t id, uint8_t toState, uint8_t fromState){
	if (!id)return;
	if (GetTaskState(id) == fromState)SetTaskState(id, toState);
}	
void Top(){
	PrintStr(" PID Name         Size    CS      IP      State\r\n", LBLUE);
	for (int i = 0;i < MaxRunNum;++i){
		LoadPCB(i);
		if (_p.STATE == T_EMPTY)continue;
		int count = 0;
		PrintChar(' ');
		count = PrintNum(_p.ID);
		for (int i = count;i < 4;++i)PrintChar(' ');
		if (i == 0){
			PrintStr("Mirai-Shell", CYAN);
		}else{
			for (count = 0;count < 11 && _p.NAME[count] != ' ';++count){
				PrintChar(_p.NAME[count], CYAN);
			}
		}
		PrintStr("  ");
		count = PrintNum(_p.SIZE);
		for (int i = count;i < 8;++i)PrintChar(' ');
		//count = PrintNum(_p.CS);
		//for (int i = count;i < 8;++i)PrintChar(' ');
		PrintStr("0x");
		PrintHex((_p.CS >> 8) & 0xFF);
		PrintHex((_p.CS) & 0xFF);
		PrintStr("  ");

		PrintStr("0x");
		PrintHex((_p.IP >> 8) & 0xFF);
		PrintHex((_p.IP) & 0xFF);
		PrintStr("  ");

		switch (_p.STATE){
			case T_RUNNING:
				PrintStr("Running",LGREEN);
				break;
			case T_READY:
				PrintStr("Ready",LRED);
				break;
			case T_SUSPEND:
				PrintStr("Suspend",LBLUE);
				break;
		}
		PrintStr(NEWLINE);
	}
}
#endif
