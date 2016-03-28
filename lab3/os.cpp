asm(".code16gcc\n");
//asm("jmp 0:main");
#include <stdint.h>
#include "include/io.h"
const char *OS_INFO = "MiraiOS 0.1";
const char *PROMPT_INFO = "wkcn > ";

const osi maxBufSize = 128;
char buf[maxBufSize]; // 指令流
osi bufSize = 0;


extern "C" void RunProg(osi);
extern "C" void KillProg(osi);

void Execute(){
	if (bufSize <= 0)return;
	char c = buf[0];
	if (c >= '0' && c <= '9'){
		RunProg(c - '0' + 10);
	}else{
		if (c == 'k'){
			KillProg(buf[2] - '0');
		}
	}
}

int main(){ 
	CLS();
	DrawText(OS_INFO,0,0,LGREEN);
	SetCursor(1,0);
	//DrawText(PROMPT_INFO,3,0,WHITE);
	while(1){
		PrintStr(PROMPT_INFO,LCARM);
		bufSize = 0; // clean buf
		while(1){
			char c = getchar();
			 if (c == '\r'){
				PrintStr(NEWLINE);
				Execute();
				break;
			}else if (c == '\b'){
				if (bufSize > 0){
					PrintChar('\b');
					PrintChar(' ');
					PrintChar('\b');
					buf[--bufSize] = 0;
				}
			}else {
				if (bufSize < maxBufSize){
					PrintChar(c, WHITE);
					buf[bufSize++] = c;
				}
 			}
 		}
 	} 
	return 0;
} 
