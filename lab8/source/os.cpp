#include "io.h"
#include "string.h"
#include "disk.h"
#include "keyboard.h"
#include "version.h"
#include "pcb.h"
#include "interrupt.h"
#include "os_sem.h"
//#include "os_msg.h"
#include "port.h"
#include "mem_base.h"
#include "prog.h"
#include "screen.h"

const uint8_t UserNum = 4;
uint16_t cursors[UserNum];

const uint16_t talkBufSize = 128;
char talkBuffer[talkBufSize];
ReadyProg readyProg;
uint16_t RunProgRetn;

//Kernel_Memory
const uint16_t MaxBlockNum = 127;
const uint16_t SPACE_SIZE = 0x4000; // 这里用段表示
MemBlock memdata[MaxBlockNum + 1];
MemRecord memRecord;


extern "C" uint16_t RunNum;
extern "C" uint8_t UserID;
extern "C" const uint8_t INT09H_FLAG;

char shellName[12] = "SHELL   COM";

__attribute__((regparm(2)))
void TabUser(uint8_t uid){
	if (!(uid >= 1 && uid <= 4))return;
	//uid = 1, 2, 3, 4, 5
	//切换用户
	SaveScreen(UserID);
	LoadScreen(uid);
	//Save Cursor
	cursors[UserID - 1] = GetCursor();
	//Load Cursor
	SetCursor(cursors[uid-1]>>8, cursors[uid-1]&0xFF);		
	UserID = uid; 
	Schedule;
}

__attribute__((regparm(2)))
uint16_t RunProg(char *filename, uint16_t allocatedSize = 0){
	if (RunNum >= MaxRunNum)return 0;
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

	uint8_t pcbID = FindEmptyPCB();

	if (!pcbID)return 0;
	_p.ID = pcbID; 
	_p.UID = UserID; // 没有使用readyprog的uid !
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

void int_23h(){
	CPP_INT_HEADER;
	/*
	 * ah = 00h, 释放段地址bx， 段大小为cx的内存
	 * ah = 01h, 申请段大小为cx的内存， 返回值为段地址
	 * ah = 10h, MEM()
	 * ah = 20h, SetShellMode = al
	 * ah = 21h, TabUser
	 */
	uint16_t ax, bx, cx, dx;
	asm volatile("":"=a"(ax),"=b"(bx),"=c"(cx),"=d"(dx));
	uint16_t ah = (ax & 0xFF00) >> 8;
	switch(ah){
		case 0x00:
			//释放内存
			mem_free(memRecord,bx,cx);	
			break;
		case 0x01:
			bx = mem_allocate(memRecord,cx);
			asm volatile("mov ax, bx;"::"b"(bx));
			break;
		case 0x10:
			MEM();
			break;
		case 0x20:
			dx = ax & 0xFF;
			WritePort(SHELLMODE_PORT, &dx, sizeof(dx));
			break;
		case 0x21:
			TabUser(ax & 0xFF);
			break;
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
	uint16_t ax, bx, cx, dx;
	asm volatile("":"=a"(ax),"=b"(bx),"=c"(cx),"=d"(dx));
	uint8_t ah = (ax & 0xFF00) >> 8;
	int8_t al = (ax & 0x00FF);
	/*
	 * ah = 00h, 创建信号量，初始值为al
	 * ah = 01h, semWait
	 * ah = 02h, semSig
	 * ah = 30h, 阻塞发送 target, cs, ip 
	 * ah = 43h, 非阻塞全接收 cs, ip
	 * 以上sid为al
	 */
	uint8_t sid;
	switch(ah){
		case 0x00:
			sid = os_semCreate(al);
			asm volatile("mov ax, bx;"::"b"((uint16_t)sid));
			break;
		case 0x01:
			os_semWait(al);
			break;
		case 0x02:
			os_semSignal(al);
			break;
		case 0x04:
			os_semDel(al);
			break;
		case 0x05:
			os_semRelease(al); // 根据RunID删除
			break;
		case 0x30:
			//ax = Send(ah,bx,cx);
			break;
		case 0x43:
			//ax = IRecvAll(bx,cx);
			break;
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

//MsgPack mp;
//char hh[128];
int main(){  
	INIT_SEGMENT();
	INIT_MEMORY();
	INIT_SEM();
	//INIT_MSG();
	WriteUserINT();
	// 初始化端口
	SetPort(READYPROG_PORT,&readyProg,sizeof(ReadyProg));
	SetPort(RUNPROGRETN_PORT, &RunProgRetn, sizeof(RunProgRetn));
	PortSemCreate(RUNPROGRETN_PORT, 0);

	SetPort(TALK_PORT,talkBuffer,talkBufSize);

	// 内核进程号为0
	// Shell编号: 1, 2, 3, 4 ...
	ScheduleOFF;
	for (uint8_t i = 1;i <= UserNum;++i){
		UserID = i;
		RunProg(shellName);
	}
	//切换为用户1
	UserID = 1;
	ScheduleON;

	while(1){

		if (INT09H_FLAG){
			GlobalColor = YELLOW;
			DrawText("Ouch! Ouch!",24,65);
			GlobalColor = WHITE;
		}else{
			DrawText("           ",24,65);
		}

		if (GetPortMsgV(TALK_PORT)){
			PrintStr(talkBuffer);
			SetPortMsgV(TALK_PORT,0);
			PrintStr(NEWLINE);
		}

		if (GetPortMsgV(READYPROG_PORT)){
			if (strcmp(readyProg.filename, "SHELL   COM") == 0)RunProgRetn = 0;
			else RunProgRetn = RunProg(readyProg.filename, readyProg.allocatedSize);
			PortSemSignal(RUNPROGRETN_PORT);
			SetPortMsgV(READYPROG_PORT, 0);
		}

		/*
		bool can = IRecvAll(&mp);
		if (can){
			memcpy(GetAddr(hh), mp.data, mp.size);
			PrintStr(hh, LBLUE);
		}
		*/

	}
	return 0;
} 

