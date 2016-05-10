#include "io.h"
#include "string.h"
#include "disk.h"
#include "keyboard.h"
#include "version.h"
#include "pcb.h"
#include "port.h"
#include "interrupt.h"
#include "prog.h"

const char *OS_INFO = "MiraiOS 0.5";
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
uint16_t INT_INFO = 0;
uint16_t ShellMode = 0;
uint16_t RunNum = 0;
uint8_t INT09H_FLAG = 0;

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

void KillAll(){
	
}

void MEM(){}

void top(){}

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
	//i = 0 为命令
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

void PR(){}

char filename[12] = "        COM";
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
	}else if(CommandMatch("pr")){
		PR();
	}else if(CommandMatch("mem")){
		MEM();
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
		for (int i = 0;i < 8;++i){
			char c = buf[i];
			if (c == '.' || c == 0)break;
			if (c >= 'a' && c <= 'z')c = c - 'a' + 'A';
			filename[i] = c;
		}
		uint16_t allocatedSize = 0;
		if (parSize > 1){
			allocatedSize = GetNum(1);
		}
		if(RunProg(filename, allocatedSize)){
			ShellMode = 1;
		}else 
			PrintInfo("Command not found, Input \'help\' to get more info",RED);
	}
	bufSize = 0;
}


int main(){
	cls();
	uname();
	DrawText("You can input \'help\' to get more info",1,0,LGREEN);	
	SetCursor(2,0);
	while(1){
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
			if (RunNum <= 1){
				ShellMode = 0;
			}
			continue;
		}

		//非Shell
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

		// 进入输入行命令模式
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
