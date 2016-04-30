#ifndef _TASK_H_
#define _TASK_H_

#include "pcb.h"
#include "memory.h"

uint8_t fork(){
	asm volatile("int 0x21;int 0x08;"::"a"(0x0700)); // 关闭进程切换, 更新PCB值
	INIT_SEGMENT();
	uint16_t runid;
	asm volatile("int 0x21;":"=a"(runid):"a"(0x0200));
	LoadPCB(runid); // note:IP!
	if (_p.KIND == K_FORK){
		SetTaskAttr(runid,&_p.KIND,uint8_t(K_PROG));
		asm volatile("int 0x21;"::"a"(0x0800));
		return 0; // 子进程返回0
	}
	uint8_t newID = FindEmptyPCB();
	uint16_t addrseg = allocate(_p.SSIZE); 
	if (addrseg == 0xFFFF){
		asm volatile("int 0x21;"::"a"(0x0800)); // 开启进程切换
		return 0xFF;
	}
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
	_p.KIND = K_FORK;

	WritePCB(newID);
	asm volatile("int 0x21;"::"a"(0x0900)); // ++RunNum
	asm volatile("int 0x21;"::"a"(0x0800)); // 开启进程切换
	return newID;
}


#endif
