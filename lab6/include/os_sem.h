#ifndef _OS_SEM_H_
#define _OS_SEM_H_

#include "pcb.h"

// 汇编的cmpxchg可以原子语句
// 这里我使用屏蔽进程切换的方式
// Block链由PCB项记录BLOCK_NEXT

struct sem{
	bool used;
	uint8_t flag; // bool 也会变为1字节
	uint8_t count;
	uint8_t next;
};

const int MaxSemNum = 128;
sem sems[MaxSemNum];

__attribute__((regparm(1)))
void semWait(uint8_t sid){
	INIT_SEGMENT();
	uint16_t ax = 1;
	while(1){
		//原子交换
		asm volatile("xchg [bx],ax":"=a"(ax):"b"(&sems[sid].flag),"a"(ax));
		if (ax == 1)continue;
		break;
	}
	--sems[sid].count;
	uint16_t runid = GetRunID();
	if (sems[sid].count < 0){
		//加入链表尾部
		if (sems[sid].next == 0){
			//信号量阻塞队列为空
			sems[sid].next = runid;	
		}else{
			int nex = sems[sid].next;
			uint8_t temp;
			while(1){
				GetTaskAttr(nex,&_p.BLOCK_NEXT,temp);
				if (temp == 0)break;
				nex = temp;
			}
			SetTaskAttr(nex,&_p.BLOCK_NEXT,(uint8_t)runid);
		}
		sems[sid].flag = 0;
		//阻塞该进程
		SetTaskAttr(runid, &_p.STATE, (uint8_t)T_BLOCKED);
	}	
}

__attribute__((regparm(1)))
void semSignal(uint8_t sid){
	INIT_SEGMENT();
	uint16_t ax = 1;
	while(1){
		//原子交换
		asm volatile("xchg [bx],ax":"=a"(ax):"b"(&sems[sid].flag),"a"(ax));
		if (ax == 1)continue;
		break;
	}
	++sems[sid].count;	
	if (sems[sid].count <= 0){
		//将sems[sid].next移出
		uint8_t zid = sems[sid].next;
		uint8_t nex;
		GetTaskAttr(zid,&_p.BLOCK_NEXT,nex);
		sems[sid].next = nex;
		//已经移出来了
		SetTaskAttr(zid, &_p.STATE, (uint8_t)T_RUNNING);
	}
	sems[sid].flag = 0;
}

__attribute__((regparm(1)))
uint8_t semCreate(uint8_t count){
	for (int i = 0;i < MaxSemNum;++i){
		if (!sems[i].used){
			sems[i].used = 0;
			sems[i].count = count;
			sems[i].next = 0;
			sems[i].flag = 1;
			return i;
		}
	}
	return 0xFF;
}

void INIT_SEM(){
	for (int i = 0;i < MaxSemNum;++i){
		sems[i].used = 0;
	}
}

#endif
