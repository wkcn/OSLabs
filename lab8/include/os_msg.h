#ifndef _OS_MSG_H_
#define _OS_MSG_H_

#include "msg_base.h"
#include "pcb.h"
#include "sem.h"

const uint16_t MsgNum = 128;
Message messages[MsgNum];
uint16_t MsgSem; 

void INIT_MSG(){
	for (uint16_t i = 0;i < MsgNum;++i){
		messages[i].used = 0;
		messages[i].id = i;
	}
	MsgSem = semCreate(1);
}


//阻塞发送
//输入msgpack
__attribute__((regparm(3)))
bool Send(uint8_t target, uint16_t cs, uint16_t ip){
	//semWait(MsgSem);
	PrintStr("Send");
	uint16_t i;
	for (i = 0;i < MsgNum;++i){
		if (!messages[i].used)break;
	}
	if (i == MsgNum)return false;

	messages[i].used = true;
	messages[i].visited = false;
	messages[i].source = GetRunID();
	messages[i].target = target;
	memcpy(GetAddr(&messages[i].pack),MakeAddr(cs,ip),sizeof(MsgPack));

	//semSignal(MsgSem);

	//阻塞自己
	PrintStr("BB");
	SetTaskAttr(GetRunID(), &_p.STATE, (uint8_t)T_BLOCKED);
	Schedule;

	return true;
}

__attribute__((regparm(2)))
bool Send(uint8_t target, MsgPack *p){
	uint16_t cs;
	asm volatile("mov ax, cs;":"=a"(cs));
	return Send(target, cs, (unsigned long)p);
}

//非阻塞接收
//目前不考虑信息发送的顺序！
//返回msgpack
__attribute__((regparm(2)))
bool IRecvAll(uint16_t cs, uint16_t ip){
	//semWait(MsgSem);
	uint16_t runid = GetRunID();
	bool finded = false;
	for (uint16_t i = 0;i < MsgNum;++i){
		if (messages[i].used && messages[i].target == runid){
			finded = true;
			memcpy(MakeAddr(cs,ip), GetAddr(&messages[i].pack), sizeof(MsgPack));
			//恢复运行态
			SetTaskAttr(messages[i].source, &_p.STATE, (uint8_t)T_RUNNING);
			messages[i].used = false;
			//暂时不考虑visited处理
			break;
		}
	}
	//semSignal(MsgSem);
	return finded;
}

__attribute__((regparm(1)))
bool IRecvAll(MsgPack *p){
	uint16_t cs;
	asm volatile("mov ax, cs;":"=a"(cs));
	return IRecvAll(cs, (unsigned long)p);
}
#endif
