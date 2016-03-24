asm(".code16gcc\n");
asm("jmp 0:main");
#include <stdint.h>
#include "io.h"
const char *OS_INFO = "MiraiOS 0.1";
const char *PROMPT_INFO = "wkcn > ";

int main(){ 
	CLS();
	DrawText(OS_INFO,0,0,LGREEN);
	SetCursor(1,0);
	//DrawText(PROMPT_INFO,3,0,WHITE);
	while(1){
		PrintStr(PROMPT_INFO,LCARM);
		while(1){
			char c = getchar();
			if (c == '\r'){
				PrintStr(NEWLINE);
				break;
			}else if (c == '\b'){
				PrintChar('\b');
				PrintChar(' ');
				PrintChar('\b');
			}else {
				PrintChar(c, WHITE);
			}
		}
	}
	return 0;
}
