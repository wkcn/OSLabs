asm(".code16gcc\n");
//asm("jmp 0:main");
#include <stdint.h>
#include "include/io.h"
#include "include/string.h"
const char *OS_INFO = "MiraiOS 0.1";
const char *PROMPT_INFO = "wkcn > ";
const char *HELP_INFO = "\
Input a 1~5 for parallel running. A stream nums for serial running \n\r\
Commands:\n\r\
r        Go to look user processes\n\r\
ls       list all programs\n\r\
cls      Clear Screen\n\r\
top      View all running processes\n\r\
kill     Kill a process, ex: kill 3\n\r\
killall  Kill all Processes\n\r\
uname    Show os info\n\r\
Keys:\n\r\
Esc      Back to Shell but not kill processes\n\r\
Ctrl+Z   Back to Shell and kill all processes\n\r\
";
const char *NOPROG_INFO = "No User Process is Running!";
const char *BATCH_INFO = "Batching Next Program: ";
const char *LS_INFO = "Please Input These Number to Run a Program or more :-)\n\r1,2,3,4 - 45 angle fly char\n\r5 Draw my name";

struct Prog{
	char name[8];
	osi space;
	char pos[8];
	char describe[32];
};

const osi progsNum = 5;
Prog progs[progsNum] = {
	{"1",512,"/","Quad 1 45-angle char"},
	{"2",512,"/","Quad 2 45-angle char"},
	{"3",512,"/","Quad 3 45-angle char"},
	{"4",512,"/","Quad 4 45-angle char"},
	{"5",512,"/","Print My Name"}
};

const osi maxBufSize = 128;
char buf[maxBufSize]; // 指令流
osi bufSize = 0;
osi par[16][2];
osi parSize = 0;
osi batchList[5] = {5,1,2,3,4};
osi batchID = 0;
osi batchSize = 0;


extern "C" uint16_t GetKey();
extern "C" void RunProg(osi);
extern "C" void KillProg(osi);
extern "C" uint16_t ShellMode;
extern "C" uint16_t RunID;
extern "C" uint16_t RunNum;

void KillAll(){
	RunNum = 1;
	RunID = 0;
	ShellMode = 0;
}

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

void LS(){
	//PrintInfo(LS_INFO,WHITE);
	PrintStr("Name Size  Pos   Description",LBLUE);
	PrintStr(NEWLINE,WHITE);
	for (osi i = 0;i < progsNum;++i){
		PrintStr(progs[i].name,WHITE);
		PrintStr("    ",WHITE);
		PrintNum(progs[i].space);
		PrintStr("    ",WHITE);
		PrintStr(progs[i].pos,WHITE);
		PrintStr("    ",WHITE);
		PrintStr(progs[i].describe,WHITE);
		PrintStr(NEWLINE,WHITE);
	}
	
}

void Top(){
	//Sorry, I have no time to finish viewing all programs pid :-(
	//I will finish it in the future~
	PrintNum(RunNum - 1,WHITE);
	PrintStr(" User Progresses are running :-)",WHITE);
	PrintStr(NEWLINE,WHITE);
}

void Execute(){  
	if (bufSize <= 0)return;
	batchSize = 0;
	batchID = 0;
	for (osi i = 0;i < bufSize && batchSize < 5;++i){
		char c = buf[i];
		osi y = c - '0';
		if (y >= 1 && y <= 5){
			batchList[batchSize++] = y;
		}
	}
	if (batchSize == 1){
		batchSize = 0;
	}
	if (batchSize >= 2){
		return;
	}
	buf[bufSize] = ' ';
	//以空格为分隔符号,最多十六个参数
	osi i,j;
	i = 0; j = 0;
	while (i < 16 && j < bufSize){
		for (;buf[j] == ' ' && j < bufSize;++j){
			buf[j] = 0;
		}
		par[i][0] = j;
		for (;buf[j] != ' ' && j < bufSize;++j);
		if (buf[j] == ' ')buf[j] = 0;
		par[i][1] = j;
		if (par[i][1] <= par[i][0])break;
		i++;
		parSize = i;
	}
	if (CommandMatch("uname")){
		PrintInfo(OS_INFO,WHITE);
	}else if (CommandMatch("help")){
		PrintStr(HELP_INFO,WHITE);
	}else if (CommandMatch("ls")){
		LS();
	}else if (CommandMatch("cls")){
		CLS();
	}else if (CommandMatch("killall")){
		KillAll();
	}else if (CommandMatch("r")){
		if(RunNum > 1){
			ShellMode = 1;
			CLS();
		}else{
			PrintInfo(NOPROG_INFO, RED);
		}
	}else if (CommandMatch("kill")){
		for (osi k = 1;(k < parSize) && IsNum(k);++k){
			KillProg(GetNum(k));
		}
	}else if (IsNum(0)){
		/*
		for (osi k = 0;(k < parSize) && IsNum(k);++k){
			osi y = GetNum(k);
			if (y >= 1 && y <= 5)
				RunProg(y);
		}*/
		for (osi k = 0;k < parSize && buf[k];++k){
			char c = buf[k];
			osi y = c - '0';
			if (y >= 1 && y <=5){
				RunProg(y);
			}
		}
		CLS();
		ShellMode = 1;
	}else if (CommandMatch("top")){
		Top();
	}else{
		PrintInfo("Command not found, Input \'help\' to get more info",RED);
	}
	bufSize = 0;
}

void sleep(){
	osi temp = 0;
	while(temp < 10000){
		osi ut = 10000;
		while (ut > 0){
			--ut;
		}
		++temp;
	}
}

int main(){  
	CLS();
	//PrintNum(1);
	//PrintNum(10);
	//PrintNum(13);
	//PrintNum(0);
	DrawText(OS_INFO,0,0,LGREEN);
	DrawText("You can input \'help\' to get more info",1,0,LGREEN);	
	SetCursor(2,0);
	//DrawText(PROMPT_INFO,3,0,WHITE);
	 while(1){
		//Tab
		uint16_t key = GetKey();
		//ShellMode = 0时, 为Shell操作
		if (ShellMode){
			//ShellMode = 1时, 切换到程序执行
			if (key == 0x2c1a || key == 0x011b){
				ShellMode = 0;
				CLS();
				if (key == 0x2c1a)//Ctrl + Z
				{
					//KillProg(RunID);
					KillAll();
				}
			}
			continue;
		}
		//非Shell
		//
		if (batchSize > 0 && batchID < batchSize){
			PrintStr(BATCH_INFO,YELLOW);
			osi id = batchList[batchID++];
			PrintChar(id + '0',YELLOW);
			sleep();
			CLS();
			RunProg(id);
			ShellMode = 1;
			continue;
		}

		PrintStr(PROMPT_INFO,LCARM);
		buf[0] = 0;
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
					buf[bufSize] = 0;
				}
 			}
 		}
 	}  
	return 0;
} 
