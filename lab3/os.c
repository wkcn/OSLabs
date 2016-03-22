asm("jmp 0:main");
#include <stdint.h>


void main(){
	uint16_t ax = 0xB800;
	uint16_t cx = 0x0700 | 'w';
	asm("mov es, ax;"
		"mov es:[0], cx;":
		:"a"(ax),"c"(cx)
		);
	while(1){
		asm("mov ax,2");
	}
}
