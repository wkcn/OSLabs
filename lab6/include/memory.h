#ifndef _MEMORY_H_
#define _MEMORY_H_

#include <cstddef>
#include "mem_base.h"
#include "pcb.h"

struct MemAllocRecord{
	bool used;
	uint16_t p;
	uint16_t size;
};

#define mem_null ((void*)0xFFFF)
const uint16_t MaxBlockNum = 32;
const uint16_t MaxAllocNum = 256;
const uint16_t SPACE_SIZE = 0x4000; // 16k, 这里用字节表示
//Prog Memory
MemAllocRecord mar[MaxAllocNum];
MemBlock memdata[MaxBlockNum + 1];
MemRecord memRecord;

__attribute__((regparm(1)))
uint16_t allocate(uint16_t needSize){
	asm volatile("int 0x21;"::"a"(0x0700)); // 关闭进程切换, 更新PCB值
	uint16_t addr;
	asm volatile(
			"int 0x23;"
			:"=a"(addr)
			:"a"(0x0100),"c"(needSize)
			);

	asm volatile("int 0x21;"::"a"(0x0800)); // 开启进程切换
	return addr;
}  

__attribute__((regparm(2)))
void free(uint16_t addr, uint16_t freeSize){
	asm volatile("int 0x21;"::"a"(0x0700)); // 关闭进程切换, 更新PCB值
	asm volatile(
			"int 0x23;"
			:
			:"a"(0x0000),"b"(addr),"c"(freeSize)
			);
	asm volatile("int 0x21;"::"a"(0x0800)); // 开启进程切换
}


//需要手动开启
void mem_init(){
	INIT_SEGMENT();
	memRecord.data = memdata;
	memRecord.MaxBlockNum = MaxBlockNum;
	uint16_t runid;
	asm volatile("int 0x21;":"=a"(runid):"a"(0x0200));
	uint16_t si, SSIZE;
	GetTaskAttr(runid, &_p.SIZE, si);
	GetTaskAttr(runid, &_p.SSIZE, SSIZE);
	uint16_t code = ((si + 0x100 + (1<<4) - 1) >> 4) << 4; 
	mem_init(memRecord,code,SSIZE<<4);
	for (int i = 0;i < MaxAllocNum;++i)mar[i].used = false;
}

void * operator new(size_t size){
	uint16_t p = mem_allocate(memRecord,size);
	if (p != 0xFFFF){
		int i;
		for (i = 0;i < MaxAllocNum;++i){
			if (!mar[i].used)break;
		}
		mar[i].used = true;
		mar[i].p = p;
		mar[i].size = size;	
	}
	return (void*)(unsigned int)p;
}

void * operator new[](size_t size){
	uint16_t p = mem_allocate(memRecord,size);
	if (p != 0xFFFF){
		int i;
		for (i = 0;i < MaxAllocNum;++i){
			if (!mar[i].used)break;
		}
		mar[i].used = true;
		mar[i].p = p;
		mar[i].size = size;	
	}
	return (void*)(unsigned int)p;
}

void operator delete(void *p){
	for (int i = 0;i < MaxAllocNum;++i){
		if (mar[i].used && mar[i].p == (uint16_t)(unsigned long)p){
			mar[i].used = false;
			mem_free(memRecord, mar[i].p, mar[i].size);
			return;
		}
	}
}

void operator delete[](void *p){
	for (int i = 0;i < MaxAllocNum;++i){
		if (mar[i].used && mar[i].p == (uint16_t)(unsigned long)p){
			mar[i].used = false;
			mem_free(memRecord, mar[i].p, mar[i].size);
			return;
		}
	}
}

#endif
