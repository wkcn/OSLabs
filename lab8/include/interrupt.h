#ifndef _INTERRUPT_H_
#define _INTERRUPT_H_
#include "defines.h"
#include "io.h"
//#include <stdint.h>

//一般C++函数, 会首先将ebp入栈, 最终恢复
//G++编译出来的压栈都是4字节的(32 bits), 但是我们的nasm是16位的:-(
//16位G++的内嵌汇编的iret操作的是EFLAGS,ECS,EIP; 因此, 我们要将可爱的flags,cs,ip转为32位~
//G++里内嵌汇编的iret为66cf, 内嵌的中断调用为16位int
//nasm的16位iret为cf
//不带局部变量的函数
//
#define CPP_INT_END asm volatile("pop ds;pop ebp;.BYTE 0xcf")
//带局部变量的函数(ebp保存原来的esp)
//leave恢复esp(用ebp做备份), 计数 +8 -6
//push ebp; mov ebp, esp; esp被修改, ebp没有被改
//注意， 这时的中断用的栈是用户栈
#define CPP_INT_LEAVE asm volatile("pop ds;leave;.BYTE 0xcf")
#define CPP_INT_HEADER asm volatile("push ds;push ax;mov ax, cs; mov ds, ax;pop ax;")
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
