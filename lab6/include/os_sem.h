#ifndef _OS_SEM_H_
#define _OS_SEM_H_

#include "pcb.h"

// 汇编的cmpxchg可以原子语句
// 这里我使用屏蔽进程切换的方式
// Block链由PCB项记录BLOCK_NEXT

struct sem{
	uint8_t used;
	uint8_t flag; // bool 也会变为1字节
	int8_t count;
	uint8_t next;
};

const uint8_t MaxSemNum = 128;
sem sems[MaxSemNum];

uint8_t temp;
__attribute__((regparm(1)))
void semWait(uint8_t sid){
	//INIT_SEGMENT();
	uint16_t ax = 1;
	while(1){
		//原子交换
		uint16_t cs;
		asm volatile("mov ax, cs;":"=a"(cs));
		asm volatile("push es;mov es, cx;"
					"lock xchg es:[bx],al;"
					"pop es;"
					:"=a"(ax):"b"(&sems[sid].flag),"a"(ax),"c"(cs));
		if ((ax&0x00FF) == 1)continue;
		break;
	}
	--sems[sid].count;
	//PrintNum(sems[sid].count,YELLOW);
	uint16_t runid = GetRunID();
	if (sems[sid].count < 0){
		//PrintStr("BLOCK");
		//加入链表尾部
		if (sems[sid].next == 0){
			//信号量阻塞队列为空
			sems[sid].next = (uint8_t)runid;	
		}else{
			uint8_t nex = sems[sid].next;
			while(1){
				GetTaskAttr(nex,&_p.BLOCK_NEXT,temp);
				if (temp == 0)break;
				nex = temp;
			}
			SetTaskAttr(nex,&_p.BLOCK_NEXT,(uint8_t)runid);
		}
		//阻塞该进程
		SetTaskAttr(runid, &_p.STATE, (uint8_t)T_BLOCKED);
	}	
	sems[sid].flag = 0;
	//防止无法及时调度, 暂时没有其他方法解决， 可能是调度程序的问题
	while(1){
		if (GetTaskState(runid) != T_BLOCKED)break;
	}
	Schedule;
}

__attribute__((regparm(1)))
void semSignal(uint8_t sid){
	//INIT_SEGMENT();
	uint16_t ax = 1;
	while(1){
		//原子交换
		uint16_t cs;
		asm volatile("mov ax, cs;":"=a"(cs));
		asm volatile("push es;mov es, cx;"
				"lock xchg es:[bx],al;"
				"pop es;":"=a"(ax):"b"(&sems[sid].flag),"a"(ax),"c"(cs));
		if ((ax&0x00FF) == 1)continue;
		break;
	}
	++sems[sid].count;	
	//PrintNum(sems[sid].count,LRED);
	if (sems[sid].count <= 0){
		//将sems[sid].next移出
		uint8_t zid = sems[sid].next;
		GetTaskAttr(zid,&_p.BLOCK_NEXT,temp);
		SetTaskAttr(zid,&_p.BLOCK_NEXT,uint8_t(0));
		sems[sid].next = temp;
		//已经移出来了
		SetTaskAttr(zid, &_p.STATE, (uint8_t)T_RUNNING);
	}
	sems[sid].flag = 0;
}

__attribute__((regparm(1)))
uint8_t semCreate(int8_t count){
	for (uint8_t i = 0;i < MaxSemNum;++i){
		if (!sems[i].used){
			sems[i].used = 1;
			sems[i].count = count;
			sems[i].next = 0;
			sems[i].flag = 0;
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
