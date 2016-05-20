#include "io.h"
#include "disk_op.h"
#include "string.h"
#include "keyboard.h"
#include "version.h"
#include "pcb.h"
#include "interrupt.h"
#include "port.h"
#include "prog.h"


const char *OS_INFO = "MiraiOS 0.7";
const char *PROMPT_INFO = "wkcn > ";
const char *NOPROG_INFO = "No User Process is Running!";
const char *BATCH_INFO = "Batching Next Program: ";
const char *LS_INFO = "Please Input These Number to Run a Program or more :-)\n\r1,2,3,4 - 45 angle fly char\n\r5 Draw my name";
const char TXT_NAME[5] = ".txt";

uint16_t ShellMode = 0;
uint8_t UserID;
uint16_t RunNum = 0;

const uint16_t maxBufSize = 1024;
char buf[maxBufSize]; // 指令流

const uint16_t talkBufSize = 128;
char talkBuffer[talkBufSize];
int bufSize = 0;
int par[16][2];
int parSize = 0;
int batchList[5] = {5,1,2,3,4};
int batchID = 0;
int batchSize = 0;

ReadyProg readyprog;

//文件系统, 这里暂时由用户程序处理，没有使用微内核技术
File file;

void UpdateRunNum(){
	RunNum = 0;
	for (uint8_t t = 0;t < MaxRunNum;++t){
		if (GetTaskState(t) == T_EMPTY)continue;
		uint8_t uid;
		GetTaskAttr(t, &_p.UID, uid);
		if (uid != UserID)continue;
		++RunNum;
	}
}

void KillAll(){
	uint8_t uid;
	for (uint8_t i = 1;i < MaxRunNum;++i){
		GetTaskAttr(i, &_p.UID, uid);
		if (uid != UserID)continue;
		GetTaskAttr(i, &_p.ID, uid);
		if (uid == UserID)continue;
		uint8_t state = GetTaskState(i);
		if (state != T_EMPTY){
			SetTaskState(i, T_DEAD);
		}
	}
}

__attribute__((regparm(2)))
void SetAllTask(uint8_t toState,uint8_t fromState){
	uint8_t uid;
	for (int i = 1;i < MaxRunNum;++i){
		GetTaskAttr(i, &_p.UID, uid);
		if (uid != UserID)continue;
		GetTaskAttr(i, &_p.ID, uid);
		if (uid == UserID)continue;
		if (GetTaskState(i) == fromState){
			SetTaskState(i, toState);
		}
	}
}

__attribute__((regparm(2)))
uint16_t RunProg(char *filename, uint16_t allocatedSize = 0){
	for (int i = 0;i < 11;++i)readyprog.filename[i] = filename[i];
	readyprog.allocatedSize = allocatedSize;
	readyprog.uid = UserID;
	WritePort(READYPROG_PORT, (void*)&readyprog, sizeof(ReadyProg));
	SetPortMsgV(READYPROG_PORT,1);
	uint16_t rv = 0;
	PortSemWait(RUNPROGRETN_PORT);
	ReadPort(RUNPROGRETN_PORT, &rv, sizeof(rv));
	return rv;
}


char RPfilename[12];
__attribute__((regparm(1)))
int RunProg(int i){
	if (i == 5){
		//char f[12] = "KAN     COM";
		memcpy(RPfilename,"KAN     COM",11);
		return RunProg(RPfilename);
	}
	//char filename[12] = "WKCN1   COM";
	memcpy(RPfilename,"WKCN1   COM",11);
	RPfilename[4] = i + '0';
	cls();
	SetAllTask(T_RUNNING, T_SUSPEND);
	return RunProg(RPfilename);
}

void MEM(){
	asm volatile("int 0x23"::"a"(0x1000));
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

void PR(){
	//pr id value
	uint8_t id = GetNum(1);
	if (GetTaskState(id) != T_EMPTY){
		uint8_t value = GetNum(2);
		if (value > 10)value = 10;
		SetTaskAttr(id,&_p.PRIORITY,value);
	}
}

void PrintUID(){
	PrintStr("UserID: ");
	PrintNum(UserID);
	PrintStr(NEWLINE);
}

__attribute__((regparm(1)))
void top(bool all = false){
	uint16_t rn = 0;
	for (uint8_t t = 0;t < MaxRunNum;++t){
		if (GetTaskState(t) == T_EMPTY)continue;
		if (!all){
			uint8_t uid;
			GetTaskAttr(t, &_p.UID, uid);
			if (uid != UserID)continue;
		}
		++rn;
	}
	
	PrintStr(" There are ");
	PrintNum(rn,WHITE);
	PrintStr(" Progresses :-)",WHITE);
	PrintStr(NEWLINE,WHITE);
	PrintStr(" PID Name         PR  Size    SEG     CS      IP      Parent  State\r\n", LBLUE);
	for (uint16_t t = 0;t < MaxRunNum;++t){
		LoadPCB(t);
		if (_p.STATE == T_EMPTY)continue;
		if (_p.UID != UserID && !all)continue;
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
		count = PrintNum(_p.PRIORITY);
		for (int i = count;i < 4;++i)PrintChar(' ');

		count = PrintNum(_p.SIZE);
		for (int i = count;i < 8;++i)PrintChar(' ');

		PrintStr("0x");
		PrintHex2(_p.SEG);
		PrintStr("  ");

		PrintStr("0x");
		PrintHex2(_p.CS);
		PrintStr("  ");

		PrintStr("0x");
		PrintHex2(_p.IP);
		PrintStr("  ");

		count = PrintNum(_p.PARENT_ID);
		for (int i = count;i < 8;++i)PrintChar(' ');

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
			case T_DEAD:
				PrintStr("Dead",RED);
				break;
			case T_BLOCKED:
				PrintStr("Blocked",YELLOW);
				break;
		}
		PrintStr(NEWLINE);
	}
}


char Exfilename[12];
char rwfilename[12];
char tarfilename[12];
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
		top(false);
	}else if (CommandMatch("topa")){
		top(true);
	}else if (CommandMatch("uid")){
		PrintUID();
	}else if (CommandMatch("cls")){
		cls();
	}else if (CommandMatch("r")){
		uint16_t RunNum;
		GetRunNum;
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
	}else if (CommandMatch("cls")){
		cls();
	}else if (CommandMatch("r")){
		uint16_t RunNum;
		GetRunNum;
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
	}else if(CommandMatch("rm")){
		uint16_t i = 0;
		for (;i < 8;++i){
			char c = buf[i + par[1][0]];
			if (c == '.' || c == 0)break;
			rwfilename[i] = c;
		}
		memcpy(rwfilename + i, TXT_NAME, sizeof(TXT_NAME));
		if(!rm(rwfilename))PrintStr("File Not Found:-( \r\n",RED);
	}else if(CommandMatch("mv")){
		uint16_t i;
		for (i = 0;i < 8;++i){
			char c = buf[i + par[1][0]];
			if (c == '.' || c == 0)break;
			rwfilename[i] = c;
		}
		memcpy(rwfilename + i, TXT_NAME, sizeof(TXT_NAME));

		for (i = 0;i < 8;++i){
			char c = buf[i + par[2][0]];
			if (c == '.' || c == 0)break;
			tarfilename[i] = c;
		}
		memcpy(tarfilename + i, TXT_NAME, sizeof(TXT_NAME));

		if(!mv(rwfilename, tarfilename))PrintStr("File Not Found:-(\r\n",RED);
	}else if(CommandMatch("cp")){
		uint16_t i;
		for (i = 0;i < 8;++i){
			char c = buf[i + par[1][0]];
			if (c == '.' || c == 0)break;
			rwfilename[i] = c;
		}
		memcpy(rwfilename + i, TXT_NAME, sizeof(TXT_NAME));

		for (i = 0;i < 8;++i){
			char c = buf[i + par[2][0]];
			if (c == '.' || c == 0)break;
			tarfilename[i] = c;
		}
		memcpy(tarfilename + i, TXT_NAME, sizeof(TXT_NAME));

		if(!cp(rwfilename, tarfilename))PrintStr("Copy Fail:-(\r\n",RED);
	}else if(CommandMatch("open")){
		uint16_t i = 0;
		for (;i < 8;++i){
			char c = buf[i + par[1][0]];
			if (c == '.' || c == 0)break;
			rwfilename[i] = c;
		}
		memcpy(rwfilename + i, TXT_NAME, sizeof(TXT_NAME));

		if (open(&file, rwfilename)){
			PrintStr("Open File: ");
			PrintStr(rwfilename);
			PrintStr(" successfully!\r\n");
		}else{
			PrintStr(rwfilename, LGREEN);
			PrintStr(" not found, create it when writting :-) \r\n",LGREEN);
		}
	}else if(CommandMatch("write")){
		if (write(&file, buf + par[1][0], bufSize - par[1][0])){
			PrintStr("Written :-)\r\n",LGREEN);
		}else{
			PrintStr("Written Failed :-(\r\n",RED);
		}
	}else if(CommandMatch("read")){
		uint16_t readSize;
		readSize = read(&file, buf, maxBufSize);
		if (readSize){
			for (uint16_t i = 0;i < readSize;++i){
				PrintChar(buf[i]);
			}
			PrintStr(NEWLINE);
		}else{
			if (file.filename[0]){
				PrintStr("You have read all words EOF! :-(\r\n",RED);
			}else{
				PrintStr("No opened file :-(\r\n",RED);
			}
		}
	}else if(CommandMatch("file")){
			if (file.filename[0]){
				PrintStr("Filename: ");
				PrintStrN(file.filename, 11);
				PrintStr("   size: ", LBLUE);
				PrintNum(size(&file), LBLUE);
				PrintStr("   g: ", LGREEN);
				PrintNum(tellg(&file), LGREEN);
				PrintStr("   p: ", LCARM);
				PrintNum(tellp(&file), LCARM);
				PrintStr(NEWLINE);
			}else{
				PrintStr("No opened file:-(\r\n",RED);
			}
	}else if(CommandMatch("seekg")){
		seekg(&file, GetNum(1));
	}else if(CommandMatch("seekp")){
		seekp(&file, GetNum(1));
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
		memcpy(Exfilename, "        COM",11);
		for (int i = 0;i < 11;++i){
			char c = buf[i];
			if (c == '.' || c == 0)break;
			if (c >= 'a' && c <= 'z')c = c - 'a' + 'A';
			Exfilename[i] = c;
		}
		uint16_t allocatedSize = 0;
		if (parSize > 1){
			allocatedSize = GetNum(1);
		}
		if(RunProg(Exfilename, allocatedSize)){
			ShellMode = 1;
		}else 
			PrintInfo("Command not found, Input \'help\' to get more info",RED);
	}
	bufSize = 0;
}

__attribute__((regparm(1)))
bool JudgeKey(uint16_t key){
	if (key == KEY_CTRL_C){
		cls();
		return true;
	}
	if (key >= KEY_ALT_1 && key <= KEY_ALT_4){
		uint16_t uid = ((key - KEY_ALT_1) >> 8) + 1;
		asm volatile("int 0x23;"::"a"(0x2100 | uid));
		return true;
	}	
	return false;
}

int main(){
	INIT_SEGMENT();
	UserID = GetRunID();
	cls();
	uname();
	GlobalColor = LGREEN;
	DrawText("You can input \'help\' to get more info",1,0);	
	GlobalColor = WHITE;
	SetCursor(2,0);
	while(1){
		uint16_t key = getkey();
		if (JudgeKey(key))continue;

		JudgeKey(key);

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


			uint16_t runningNum = 0;
			for (uint8_t t = 0;t < MaxRunNum;++t){
				TASK_STATE state = (TASK_STATE)GetTaskState(t);
				if (state != T_RUNNING && state != T_READY)continue;
				uint8_t uid;
				GetTaskAttr(t, &_p.UID, uid);
				if (uid != UserID)continue;
				++runningNum;
			}
			if (runningNum <= 1)ShellMode = 0;
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
			uint16_t key = getchar();
			if(JudgeKey(key))continue;
			char c = key & 0xFF;
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
