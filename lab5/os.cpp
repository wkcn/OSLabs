asm(".code16gcc\n");
//asm("jmp 0:main");
#include <stdint.h>
#include "include/io.h"
#include "include/string.h"
#include "include/disk.h"
#include "include/keyboard.h"
#include "include/task.h"
#include "include/version.h"
#include "include/interrupt.h"
#include "include/port.h"

const char *OS_INFO = "MiraiOS 0.2";
const char *PROMPT_INFO = "wkcn > ";
const char *NOPROG_INFO = "No User Process is Running!";
const char *BATCH_INFO = "Batching Next Program: ";
const char *LS_INFO = "Please Input These Number to Run a Program or more :-)\n\r1,2,3,4 - 45 angle fly char\n\r5 Draw my name";

const uint16_t maxBufSize = 128;
char buf[maxBufSize]; // 指令流
const uint16_t talkBufSize = 128;
char talkBuffer[talkBufSize];
int bufSize = 0;
int par[16][2];
int parSize = 0;
int batchList[5] = {5,1,2,3,4};
int batchID = 0;
int batchSize = 0;


//extern "C" void WritePCB(uint16_t addr);
extern "C" uint16_t ShellMode;
extern "C" uint16_t RunNum;
extern "C" const uint8_t INT09H_FLAG;
extern "C" uint16_t INT_INFO; //中断信号 

uint16_t PROG_SEGMENT_S = 0;

__attribute__((regparm(1)))
int RunProg(char *filename){
	if (RunNum >= MaxRunNum)return 0;
	//addr = (char*)(((PROG_SEGMENT + PROG_SEGMENT_S) << 4) + 0x100); 
	//uint16_t addrseg = (PROG_SEGMENT + PROG_SEGMENT_S); 
	uint16_t offset = 0x100;
	uint16_t addrseg = (PROG_SEGMENT + PROG_SEGMENT_S);
	int si = LoadFile(filename,offset,addrseg);
	if (si == 0)return 0;
	uint16_t SSIZE = ((si + 0x100 + (1<<4) - 1) >> 4); 
	PROG_SEGMENT_S += SSIZE; 

	//设置用户程序运行标志
	asm volatile(
			"push es;"
			"push si;"
			"push ax;"
			"mov ax, 0x00;"
			"mov es, ax;"
			"mov ax, 0x7c00;"
			"mov si, ax;"
			"mov ax, 0;"
			"mov es:[si],ax;"
			"pop ax;"
			"pop si;"
			"pop es;"
			);
	//WritePCB(addrseg);
	uint8_t pcbID = FindEmptyPCB();
	if (!pcbID)return 0;
	_p.ID = pcbID; 
	_p.CS = addrseg;
	_p.DS = addrseg;
	_p.SS = addrseg;
	_p.IP = 0x100;
	_p.SP = 0x100 - 4;
	_p.FLAGS = 512;
	_p.STATE = T_READY;
	_p.SIZE = si;
	_p.SSIZE = SSIZE; 
	_p.PARENT_ID = 0;
	_p.SEG = addrseg;

	int ni = 0;
	for (int i = 0;i < 8 && filename[i] != ' ';++i)_p.NAME[ni++] = filename[i];
	_p.NAME[ni++] = '.';
	for (int i = 8;i < 11 && filename[i] != ' ';++i)_p.NAME[ni++] = filename[i];
	for (;ni<16;++ni)_p.NAME[ni] = 0;
	WritePCB(pcbID);
	++RunNum;
	return si;
}

__attribute__((regparm(1)))
int RunProg(int i){
	if (i == 5){
		char f[12] = "KAN     COM";
		return RunProg(f);
	}
	char filename[12] = "WKCN1   COM";
	filename[4] = i + '0';
	cls();
	SetAllTask(T_RUNNING, T_SUSPEND);
	return RunProg(filename);
}
void Top(){
	PrintStr(" PID Name         Size    CS      IP      State\r\n", LBLUE);
	for (uint16_t t = 0;t < MaxRunNum;++t){
		LoadPCB(t);
		if (_p.STATE == T_EMPTY)continue;
		uint16_t count = 0;
		PrintChar(' ');
		count = PrintNum(_p.ID);
		for (uint16_t i = count;i < 4;++i)PrintChar(' ');
		if (t == 0){
			PrintStr("Mirai-Shell", CYAN);
		}else{
			for (count = 0;count < 11 && _p.NAME[count] != ' ';++count){
				PrintChar(_p.NAME[count], CYAN);
			}
		}
		PrintStr("  ");
		count = PrintNum(_p.SIZE);
		for (int i = count;i < 8;++i)PrintChar(' ');
		//count = PrintNum(_p.CS);
		//for (int i = count;i < 8;++i)PrintChar(' ');
		PrintStr("0x");
		PrintHex((_p.CS >> 8) & 0xFF);
		PrintHex((_p.CS) & 0xFF);
		PrintStr("  ");

		PrintStr("0x");
		PrintHex((_p.IP >> 8) & 0xFF);
		PrintHex((_p.IP) & 0xFF);
		PrintStr("  ");

		switch (_p.STATE){
			case T_RUNNING:
				PrintStr("Running",LGREEN);
				break;
			case T_READY:
				PrintStr("Ready",LRED);
				break;
			case T_SUSPEND:
				PrintStr("Suspend",LBLUE);
				break;
		}
		PrintStr(NEWLINE);
	}
}


void top(){
	PrintStr(" There are ");
	PrintNum(RunNum,WHITE);
	PrintStr(" Progresses :-)",WHITE);
	PrintStr(NEWLINE,WHITE);
	Top();
}

void uname(){
	PrintStr(OS_INFO,LGREEN);
	PrintStr(" #",LGREEN);
	PrintNum(RELEASE_TIMES,LGREEN);
	PrintStr(NEWLINE);
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
int GetNum(int i){
	//第一个参数 i = 1
	int j = par[i][0];
	int k = par[i][1];
	int res = 0;
	if (buf[k-1] == 'h' || buf[k-1] == 'H'){
		k--;
		for (;j<k;++j){
			char c = buf[j];
			res *= 16;
			if (c >= '0' && c <= '9'){
				res += c - '0';
			}else if (c >= 'A' && c <= 'F'){
				res += c - 'A' + 10;
			}else if (c >= 'a' && c <= 'f'){
				res += c - 'a' + 10;
			}
		}
	}else{
		for (;j<k;++j){
			char c = buf[j];
			res = res * 10 + c - '0';
		}
	}
	return res;
}

__attribute__((regparm(1)))
bool IsNum(int i){
	int j = par[i][0];
	int k = par[i][1];
	if (j >= k)return false;
	bool hex = false;
	if (buf[k-1] == 'h' || buf[k-1] == 'H'){
		hex = true;
		k--;
	}
	for (;j<k;++j){
		char c = buf[j];
		if (c < '0' || c > '9' || (hex && ((c >='a' && c<='f') || (c >= 'A' && c <= 'F'))))return false;
	}
	return true;
}

void Execute(){  
	if (bufSize <= 0)return;
	batchSize = 0;
	batchID = 0;
	for (int i = 0;i < bufSize && batchSize < 5;++i){
		char c = buf[i];
		int y = c - '0';
		if (y >= 1 && y <= 5){
			batchList[batchSize++] = y;
		}else{
			if (c != ' ')break;
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
	int i,j;
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
		++j;
		++i;
		parSize = i;
	}
	if (CommandMatch("uname")){
		uname();
	}else if (CommandMatch("top")){
		top();
	}else if (CommandMatch("cls")){
		cls();
	}else if (CommandMatch("r")){
		if(RunNum > 1){
			ShellMode = 1;
			SetAllTask(T_RUNNING,T_SUSPEND);
			cls();
		}else{
			PrintInfo(NOPROG_INFO, RED);
		}
	}else if(CommandMatch("killall")){
		KillAll();
		cls();
	}else if(CommandMatch("k") || CommandMatch("kill")){
		for(int q=1;q<parSize;++q)KillTask(GetNum(q));
	}else if(CommandMatch("wake")){
		for(int q=1;q<parSize;++q)SetTaskState(GetNum(q),T_RUNNING,T_SUSPEND);
	}else if(CommandMatch("int")){
		uint16_t id = GetNum(1);
		if (false &&  !(id >= 0x33 && id <= 0x36)){
			PrintStr("Sorry, You are allowed to use int 33 to int 36!\r\n",RED);
		}else
			ExecuteINT(id);
	}else if(CommandMatch("suspend")){
		for(int q=1;q<parSize;++q)SetTaskState(GetNum(q),T_SUSPEND,T_RUNNING);
	}else if (IsNum(0)){
		for (int k = 0;k < parSize && buf[k];++k){
			char c = buf[k];
			int y = c - '0';
			if (y >= 1 && y <=5){
				RunProg(y);
			}
		}
		//CLS();
		ShellMode = 1;
	}else{
		//Check File
		char filename[12] = "        COM";
		for (int i = 0;i < 11;++i){
			char c = buf[i];
			if (c == '.' || c == 0)break;
			if (c >= 'a' && c <= 'z')c = c - 'a' + 'A';
			filename[i] = c;
		}
		if(RunProg(filename)){
			ShellMode = 1;
		}else 
			PrintInfo("Command not found, Input \'help\' to get more info",RED);
	}
	bufSize = 0;
}

bool NeedRetnShell(){
	uint8_t a;
	asm volatile(
			"push si;"
			"push es;"
			"mov ax, 0x00;"
			"mov es, ax;"
			"mov ax, 0x7c00;"
			"mov si, ax;"
			"mov ax, es:[si];"
			"pop es;"
			"pop si;"
			:"=a"(a)
			);
	return (a && ShellMode);
}

void int_23h(){
	PrintStr("I'm an interrupt written by C++", YELLOW);
	PrintStr(NEWLINE);
	CPP_INT_END;
}

void int_24h(){
	//24h 中断
	//转换大小写
	//功能号: ah
	//偏移量: bx
	//段地址: dx
	//ah = 0, 全部转小写
	//ah = 1, 全部转大写
	int i = 0;
	uint16_t ax;
	uint16_t offset;
	uint16_t seg;
	asm volatile("":"=a"(ax),"=b"(offset),"=d"(seg));
	bool toSmall = ((ax&0xFF00) == 0);
	char c;
	asm volatile("push es;mov es,dx;"::"d"(seg));
	
	while(true){
		asm volatile(
				"mov al,es:[bx];"
				:"=al"(c)
				:"b"(offset + i)
				);
		if (c == 0)break;
		if (toSmall){
			if (c >= 'A' && c <= 'Z'){
				c = c - 'A' + 'a';
			}
		}else{
			if (c >= 'a' && c <= 'z'){
				c = c - 'a' + 'A';
			}
		}
		asm volatile(
				"mov es:[bx], al;"
				:
				:"a"(c),"b"(offset + i)
				);
		++i;
	}
	asm volatile("pop es;");
	CPP_INT_LEAVE;
}

void WriteUserINT(){
	WriteIVT(0x23,int_23h);
	WriteIVT(0x24,int_24h);
}

int main(){  
	INIT_SEGMENT();
	WriteUserINT();
	SetPort(3,&PROG_SEGMENT_S,sizeof(PROG_SEGMENT_S));
	SetPort(5,talkBuffer,talkBufSize);
	cls();
	uname();
	DrawText("You can input \'help\' to get more info",1,0,LGREEN);	
	SetCursor(2,0);
	while(1){
		if (INT_INFO >= 1 && INT_INFO <= 5){
			RunProg(INT_INFO);
			INT_INFO = 0;
			ShellMode = 1;
		}
		//Tab
		uint16_t key = getkey();
		if (key == KEY_CTRL_C){
			cls();
		}
		//ShellMode = 0时, 为Shell操作
		if (ShellMode){
			//ShellMode = 1时, 切换到程序执行
			if (key == KEY_CTRL_Z || key == KEY_ESC){
				ShellMode = 0;
				if (key == KEY_CTRL_Z){
					KillAll();
				}else{
					SetAllTask(T_SUSPEND,T_RUNNING);
				}
				cls();
			}
			if (NeedRetnShell()){
				KillAll();
			}
			if (RunNum <= 1){
				ShellMode = 0;
			}

			if (INT09H_FLAG){
				DrawText("Ouch! Ouch!",24,65,YELLOW);
			}else{
				DrawText("           ",24,65,YELLOW);
			}
			continue;
		}
		//非Shell
		//
		if (batchSize > 0 && batchID < batchSize){
			PrintStr(BATCH_INFO,YELLOW);
			int id = batchList[batchID++];
			PrintChar(id + '0',YELLOW);
			sleep(1);
			cls();
			RunProg(id);
			ShellMode = 1;
			continue;
		}

		PrintStr(PROMPT_INFO,LCARM);
		buf[0] = 0;
		bufSize = 0; // clean buf
		while(1){

			if (GetPortMsgV(5)){
				PrintStr(talkBuffer);
				SetPortMsgV(5,0);
				PrintStr(NEWLINE);
				break;
			}

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
