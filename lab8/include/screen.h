#ifndef _SCREEN_H_
#define _SCREEN_H_

__attribute__((regparm(1)))
void SaveScreen(char *data){
	asm volatile("push es;mov es, ax;"::"a"(0xB800));
	for (int i = 0;i < 80 * 25 * 2;++i){
		asm volatile("mov al, es:[bx]":"=a"(data[i]):"b"(i));
	}
	asm volatile("pop es;");
}

__attribute__((regparm(1)))
void LoadScreen(char *data){
	asm volatile("push es;mov es, ax;"::"a"(0xB800));
	for (int i = 0;i < 80 * 25 * 2;++i){
		asm volatile("mov es:[bx],al"::"a"(data[i]),"b"(i));
	}
	asm volatile("pop es;");
}

#endif
