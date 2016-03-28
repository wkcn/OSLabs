asm(".code16gcc\n");
//asm("jmp 0:main");
#include <stdint.h>
#include "include/io.h"
#include "include/string.h"
const char *OS_INFO = "MiraiOS 0.1";
const char *PROMPT_INFO = "wkcn > ";

const osi maxBufSize = 128;
char buf[maxBufSize]; // 指令流
osi bufSize = 0;
osi par[16][2];


extern "C" uint16_t GetKey();
extern "C" void RunProg(osi);
extern "C" void KillProg(osi);
extern "C" void KillAll();
extern "C" uint16_t ShellMode;

__attribute__((regparm(2)))
void PrintInfo(const char* str, uint16_t color){
	PrintStr(PROMPT_INFO,LCARM);
	PrintStr(str,color);
	PrintStr(NEWLINE,color);
}

__attribute__((regparm(1)))
bool CommandMatch(const char* str){
	return (!strcmp(buf + par[0][0], str));
}

__attribute__((regparm(1)))
osi GetNum(osi i){
	//第一个参数 i = 1
	osi j = par[i][0];
	osi k = par[i][1];
	osi res = 0;
	for (;j<k;++j){
		char c = buf[j];
		res = res * 10 + c - '0';
	}
	return res;
}

__attribute__((regparm(1)))
bool IsNum(osi i){
	osi j = par[i][0];
	osi k = par[i][1];
	if (j >= k)return false;
	for (;j<k;++j){
		char c = buf[j];
		if (c < '0' || c > '9')return false;
	}
	return true;
}

void Execute(){  
	if (bufSize <= 0)return;
	buf[bufSize] = ' ';
	//以空格为分隔符号,最多十六个参数
	osi i,j;
	i = 0; j = 0;
	while (i < 16 && j < bufSize){
		for (;buf[j] == ' ' && j < bufSize;++j);
		par[i][0] = j;
		for (;buf[j] != ' ' && j < bufSize;++j);
		buf[j] = 0;
		par[i][1] = j;
		i++;
	}
	if (CommandMatch("uname")){
		PrintInfo(OS_INFO,WHITE);
	}else if (CommandMatch("cls")){
		CLS();
	}else if (CommandMatch("killall")){
		KillAll();
	}else if (CommandMatch("r")){
		CLS();
		ShellMode = 1;
	}else if (CommandMatch("kill")){
		for (osi k = 1;k < 16 && IsNum(k);++k){
			KillProg(GetNum(k));
		}
	}else if (IsNum(0)){
		for (osi k = 0;k < 16 && IsNum(k);++k){
			osi y = GetNum(k);
			if (y >= 0 && y <= 4)
				RunProg(y + 14);
		}
	}
	else{
		PrintInfo("Command not found",RED);
	}
	bufSize = 0;
}

int main(){  
	CLS();
	//PrintNum(1);
	//PrintNum(10);
	//PrintNum(13);
	//PrintNum(0);
	DrawText(OS_INFO,0,0,LGREEN);
	SetCursor(1,0);
	//DrawText(PROMPT_INFO,3,0,WHITE);
	while(1){
		//Tab
		uint16_t key = GetKey();
		//ShellMode = 0时, 为Shell操作
		if (ShellMode){
			//ShellMode = 1时, 切换到程序执行
			if (key == 0x2c1a || key == 0x011b){
				CLS();
				ShellMode = 0;
			}
			continue;
		}
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
				if (bufSize < maxBufSize - 1){
					PrintChar(c, WHITE);
					buf[bufSize++] = c;
				}
 			}
 		}
 	}  
	return 0;
} 
