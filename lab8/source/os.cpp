#include "io.h"
#include "string.h"
#include "disk.h"
#include "keyboard.h"
#include "version.h"
#include "pcb.h"
#include "interrupt.h"
#include "port.h"
#include "mem_base.h"
#include "os_sem.h"

const uint16_t talkBufSize = 128;
char talkBuffer[talkBufSize];

//Kernel_Memory
const uint16_t MaxBlockNum = 127;
const uint16_t SPACE_SIZE = 0x4000; // 这里用段表示
MemBlock memdata[MaxBlockNum + 1];
MemRecord memRecord;


//extern "C" void WritePCB(uint16_t addr);
extern "C" uint16_t ShellMode;
extern "C" uint16_t RunNum;
extern "C" const uint8_t INT09H_FLAG;
extern "C" uint16_t INT_INFO; //中断信号 

void KillAll(){
	for (uint8_t i = 1;i < MaxRunNum;++i){
		uint8_t state = GetTaskState(i);
		if (state != T_EMPTY){
			SetTaskState(i, T_DEAD);
		}
	}
}

__attribute__((regparm(1)))
int RunProg(char *filename, uint16_t allocatedSize = 0){
	if (RunNum >= MaxRunNum)return 0;
	//uint16_t addrseg = (PROG_SEGMENT + PROG_SEGMENT_S); 
	uint16_t offset = 0x100;
	uint16_t si = GetFileSize(filename);
	// if si + allocatedSize > 0xFFFF   =>   allocatedSize > 0xFFFF - si
	if (allocatedSize > uint16_t(0xF000) - si){
		si = 0xF000;
	}else{
		si += allocatedSize;
	}
	uint16_t SSIZE = uint16_t((si + 0x100 + (1<<4) - 1) >> 4); 
	uint16_t addrseg = mem_allocate(memRecord,SSIZE);//(PROG_SEGMENT + PROG_SEGMENT_S);
	if (addrseg == 0xFFFF){
		PrintStr("Lack of Memory\r\n",RED);
		return 0;
	}
	si = LoadFile(filename,offset,addrseg);
	if (si == 0){
		return 0;
	}

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
	_p.BLOCK_NEXT = 0;
	_p.KIND = K_PROG;
	_p.PRIORITY = 0;
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

void MEM(){
	uint16_t validMem = 0;
	uint16_t maxBlock = 0;
	for (int p = memdata[0].next; p != MaxBlockNum; p = memdata[p].next){
		PrintChar('[');
		PrintNum(memdata[p].left);
		PrintStr(", ");
		PrintNum(memdata[p].right);
		PrintStr(") ");
		uint16_t u = (memdata[p].right - memdata[p].left) / (1024 / 16);
		validMem += u;
		if (u > maxBlock)maxBlock = u;
	}
	PrintStr(NEWLINE);
	PrintStr("Memory  : ",GREEN);
	PrintNum(validMem);
	PrintStr(" / ");
	PrintNum(SPACE_SIZE / (1024 / 16));
	PrintStr(" Kbytes\r\n");
	PrintStr("MaxBlock: ",GREEN);
	PrintNum(maxBlock);
	PrintStr(" Kbytes\r\n");
}

void Top(){
	PrintStr(" PID Name         PR  Size    SEG     CS      IP      Parent  State\r\n", LBLUE);
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

void int_23h(){
	CPP_INT_HEADER;
	/*
	 * ah = 00h, 释放段地址bx， 段大小为cx的内存
	 * ah = 01h, 申请段大小为cx的内存， 返回值为段地址
	 */
	uint16_t ax;
	uint16_t size;
	uint16_t addr;
	asm volatile("":"=a"(ax),"=b"(addr),"=c"(size));
	uint16_t ah = (ax & 0xFF00) >> 8;
	if (ah == 0x00){
		//释放内存
		mem_free(memRecord,addr,size);	
	}else if (ah == 0x01){
		addr = mem_allocate(memRecord,size);
		asm volatile("mov ax, bx;"::"b"(addr));
	}
	CPP_INT_LEAVE;
}

void int_24h(){
	CPP_INT_HEADER;
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


void int_25h(){
	CPP_INT_HEADER;
	asm volatile("sti;");
	uint16_t ax;
	asm volatile("":"=a"(ax));
	uint8_t ah = (ax & 0xFF00) >> 8;
	int8_t al = (ax & 0x00FF);
	/*
	 * ah = 00h, 创建信号量，初始值为al
	 * ah = 01h, semWait
	 * ah = 02h, semSig
	 * 以上sid为al
	 */
	if (ah == 0x00){
		uint8_t sid = semCreate(al);
		asm volatile("mov ax, bx;"::"b"((uint16_t)sid));
	}else if (ah == 0x01){
		semWait(al);
		//PrintStr("wait");
		//PrintNum(al);
	}else if (ah == 0x02){
		//PrintStr("sig");
		//PrintNum(al);
		semSignal(al);
	}else if (ah == 0x04){
		semDel(al);
	}else if (ah == 0x05){
		semRelease(al); // 根据RunID删除
	}
	CPP_INT_LEAVE;
}

void int_26h(){
	CPP_INT_HEADER;
	PrintStr("I'm an interrupt written by C++", YELLOW);
	PrintStr(NEWLINE);
	CPP_INT_END;
}

void WriteUserINT(){
	WriteIVT(0x23,int_23h);
	WriteIVT(0x24,int_24h);
	WriteIVT(0x25,int_25h);
	WriteIVT(0x26,int_26h);
}

void INIT_MEMORY(){
	memRecord.data = memdata;
	memRecord.MaxBlockNum = MaxBlockNum;
	mem_init(memRecord, PROG_SEGMENT, PROG_SEGMENT + SPACE_SIZE); 
}

int main(){  
	INIT_SEGMENT();
	INIT_MEMORY();
	INIT_SEM();
	WriteUserINT();
	SetPort(5,talkBuffer,talkBufSize);
	char shellName[12] = "SHELL   COM";
	RunProg(shellName);
	while(1){
		
	}
	return 0;
} 
