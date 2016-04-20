#ifndef _INTERRUPT_H_
#define _INTERRUPT_H_
#include "defines.h"
#include "io.h"
//#include <stdint.h>

//一般C++函数, 会首先将ebp入栈, 最终恢复
//G++编译出来的压栈都是4字节的(32 bits), 但是我们的nasm是16位的:-(
//16位G++的内嵌汇编的iret操作的是EFLAGS,ECS,EIP; 因此, 我们要将可爱的flags,cs,ip转为32位~
//不带局部变量的函数
#define CPP_INT_END asm volatile("sub ebp, 8;mov [ebp],ax;mov ax,[ebp+8];mov [ebp+2],ax;mov ax,[ebp+10];mov [ebp+4],ax;mov ax,[ebp+12];mov [ebp+6],ax;mov ax,[ebp+14];mov [ebp+10],ax;mov ax,[ebp+16];mov [ebp+14],ax;xor ax,ax;mov [ebp+8],ax;mov [ebp+12],ax;mov [ebp+16],ax;mov ax,[ebp];sub sp,6;pop ebp;iret;")
//带局部变量的函数(ebp保存原来的esp)
//leave恢复esp(用ebp做备份), 计数 +8 -6
#define CPP_INT_LEAVE asm volatile("sub ebp, 8;mov [ebp],ax;mov ax,[ebp+8];mov [ebp+2],ax;mov ax,[ebp+10];mov [ebp+4],ax;mov ax,[ebp+12];mov [ebp+6],ax;mov ax,[ebp+14];mov [ebp+10],ax;mov ax,[ebp+16];mov [ebp+14],ax;xor ax,ax;mov [ebp+8],ax;mov [ebp+12],ax;mov [ebp+16],ax;mov ax,[ebp];sub sp,6;leave;add esp, 2;iret;")
__attribute__((regparm(3)))
void WriteIVT(uint16_t id,uint16_t offset,uint16_t cs){
	asm volatile(
			"push si;push es;"
			"mov es, ax;"
			"mov si, bx;"
			"mov es:[si], cx;"
			"mov es:[si+2], dx;"
			"pop es;pop si;"
			:
			:"a"(0),"b"(id * 4),"c"(offset),"d"(cs)
			);
}

__attribute__((regparm(2)))
void WriteIVT(uint16_t id, void (*func)()){
	uint16_t cs;
	asm volatile("mov ax, cs;":"=a"(cs));
	WriteIVT(id,(long)func,cs);
}

__attribute__((regparm(1)))
void ExecuteINT(uint16_t id){
	uint16_t ip, cs;
	asm volatile(
			"push es;"
			"mov es, ax;"
			"mov ax, es:[bx];"
			"mov bx, es:[bx + 2];"
			"pop es;"
			:"=a"(ip),"=b"(cs)
			:"a"(0),"b"(id * 4)
			);
	uint16_t ocs;
	asm volatile("mov ax, cs;":"=a"(ocs));
	//不知道为什么, 一定要加下面这一句, 否则崩溃:-(
	PrintStr("Execute interrupt 0x");
	PrintHex(id);PrintStr(NEWLINE);
	char addr[4];
	*((uint16_t*)(addr)) = ip;
	*((uint16_t*)(addr+2)) = cs;
	asm volatile(
			"push es;"
			"mov es, ax;"
			"pushf;"
			"call far ptr es:[bx];"
			"pop es;"
			:
			:"a"(ocs),"b"(addr)
			);
}	

#endif
