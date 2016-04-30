#ifndef _MEMORY_H_
#define _MEMORY_H_

#include <stdint.h>

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

#endif
