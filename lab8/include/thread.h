#ifndef _THREAD_H_
#define _THREAD_H_

#include "pcb.h"
#include "memory.h"

struct thread_t{
	uint16_t tid;
};

uint8_t parentID; // 使用引用要放外面？
__attribute__((regparm(3)))
uint8_t thread_create(thread_t *t, __attribute__((regparm(1)))void* (*func)(void*), void *attr){
	ScheduleOFF;
	Schedule;
	INIT_SEGMENT();
	uint16_t runid = GetRunID();
	LoadPCB(runid); // note:IP!
	if (_p.KIND == K_THREAD){
		void *result = func(attr); // 执行函数
		SetTaskState(runid, T_DEAD);
		GetTaskAttr(runid, &_p.PARENT_ID, parentID);
		SetTaskAttr(parentID, &_p.STATE, uint8_t(T_RUNNING)); // 让父亲返回RUNNING态
		ScheduleON;
		asm volatile("mov ax, bx;"::"b"((uint16_t)(unsigned long)result));
		Schedule;
		while(1){}
		return 0; // 子进程返回0
	}
	uint8_t newID = FindEmptyPCB();
	uint16_t addrseg = allocate(0x10); 
	if (addrseg == 0xFFFF){
		ScheduleON;
		return 0xFF;
	}
	//[ds:si] -> [es:di]
	asm volatile("push ds;push si;push es;push di;"
			"mov ds,ax;"
			"mov es,dx;"
			"xor si,si;"
			"xor di,di;"
			"cld;"
			"COPY_STACK:;"
			"movsw;movsw;movsw;movsw;"
			"movsw;movsw;movsw;movsw;"
			"loop COPY_STACK;"
			"pop di;pop es;pop si;pop ds;"
			:
			:"a"(_p.SEG),"d"(addrseg),"c"(0x10)
			);

	// 注意, ID与RunID类型是不同的,db和dw
	_p.ID = newID;
	_p.SS = addrseg;
	_p.SEG = addrseg;
	_p.SSIZE = 0x10;
	_p.PARENT_ID = runid;
	_p.KIND = K_THREAD;
	t->tid = newID;
	WritePCB(newID);
	INC_RunNum;
	ScheduleON;
	return newID;
} 


__attribute__((regparm(2)))
uint8_t thread_join(thread_t _t, void **_thread_retn){
	INIT_SEGMENT();
	uint16_t tid = _t.tid;
	uint16_t runid = GetRunID();
	while(1){
		if (GetTaskState(tid) == T_DEAD)break;
		SetTaskAttr(runid, &_p.STATE, uint8_t(T_BLOCKED)); // 设置自己为阻塞态
		Schedule;
	}
	uint16_t ax;
	GetTaskAttr(tid, &_p.AX, ax);	
	//SetTaskAttr(tid, &_p.KIND, (uint8_t)K_PROG); // 设为普通程序， 并杀死
	if (_thread_retn)*_thread_retn = (void*)(long)ax;
	return tid;
}
#endif
