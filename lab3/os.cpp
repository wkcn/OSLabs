asm(".code16gcc\n");
asm("jmp 0:main");
#include <stdint.h>
#include "io.h"
const char *OS_INFO = "MiraiOS 0.1";
const char *PROMPT_INFO = "wkcn > ";
stream buf; // 命令流


//extern void RunProg();
void Execute(){
	//RunProg();
}

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
				Execute();
				break;
			}else if (c == '\b'){
				PrintChar('\b');
				PrintChar(' ');
				PrintChar('\b');
				buf.pop();
			}else {
				PrintChar(c, WHITE);
				buf.put(c);
 			}
 		}
 	} 
	return 0;
} 
