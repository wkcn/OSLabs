#include "disk.h"

char buf[80 * 25];
File f;
char filename[12] = "SCREEN.TXT";
int main(){
	open(&f, filename);
	asm volatile("push es;");
	asm volatile("mov es, ax;"::"a"(0xB800));
	for (int i = 0;i < 80*25;++i){
		asm volatile(
				"mov ax, es:[bx];"
				:"=a"(buf[i])
				:"b"(i * 2)
				);
	}
	asm volatile("pop es;");
	write(&f, buf, 80 * 25);
	return 0;
}
