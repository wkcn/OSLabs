#ifndef _TASK_H_
#define _TASK_H_

#include "defines.h"
#include <stdint.h>
#include "io.h"
#include "port.h"

typedef char db;
typedef uint16_t dw;
typedef uint32_t dd;
typedef uint64_t dq;

#pragma pack (1)
struct Processes{
	db ID;
	db STATE;
	db NAME[16];
	db PARENT_ID;
	db FORK_RETN;
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

__attribute__((regparm(3)))
void SetTaskSomeThing1(uint8_t id, void *something, uint8_t value){
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

uint8_t fork(){
	asm volatile("int 0x21;int 0x08;"::"a"(0x0700));
	INIT_SEGMENT();
	uint16_t runid;
	asm volatile("int 0x21;":"=a"(runid):"a"(0x0200));
	LoadPCB(runid); // note:IP!
	if (_p.FORK_RETN == runid){
		asm volatile("int 0x21;"::"a"(0x0800));
		SetTaskSomeThing1(runid,&_p.FORK_RETN,0);
		return 0; // 子进程返回0
	}
	uint8_t newID = FindEmptyPCB();
	uint16_t PROG_SEG_S;
	ReadPort(3,&PROG_SEG_S,sizeof(PROG_SEG_S));
	uint16_t addrseg = PROG_SEGMENT + PROG_SEG_S;
	PROG_SEG_S += _p.SSIZE;
	WritePort(3,&PROG_SEG_S,sizeof(PROG_SEG_S));
	//[ds:si] -> [es:di]
	asm volatile("push ds;push si;push es;push di;"
			"mov ds,ax;"
			"mov es,dx;"
			"xor si,si;"
			"xor di,di;"
			"cld;"
			"COPY_PROG:;"
			"movsw;movsw;movsw;movsw;"
			"movsw;movsw;movsw;movsw;"
			"loop COPY_PROG;"
			"pop di;pop es;pop si;pop ds;"
			:
			:"a"(_p.SEG),"d"(addrseg),"c"(_p.SSIZE)
			);

	// 注意, ID与RunID类型是不同的,db和dw
	_p.ID = newID;
	_p.CS = addrseg;
	_p.DS = addrseg;
	_p.SS = addrseg;
	_p.SEG = addrseg;
	_p.PARENT_ID = runid;
	_p.FORK_RETN = newID;

	WritePCB(newID);
	asm volatile("int 0x21;"::"a"(0x0900)); // ++RunNum
	asm volatile("int 0x21;"::"a"(0x0800)); // 开启时钟
	return newID; // 父进程返回新创建的子进程ID
}

#endif
