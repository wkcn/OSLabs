#ifndef _DISK_OP_H_
#define _DISK_OP_H_

#include "disk.h"

char fat12name[11];
char targetname[11];

__attribute__((regparm(1)))
bool rm(char *filename){
	Entry e;	
	ToFAT12Name(fat12name, filename);
	uint16_t eid = FindEntry(fat12name, &e);
	if(eid == 0xFFFF)return false;
	/*
	uint16_t cl = e.DIR_FstClus;
	uint16_t ne;
	while(cl < 0xFF8){
		ne = GetNextFat(cl);
		SetClus(cl, 0); // 设置为空
		cl = ne;
	}
	*/
	e.DIR_Name[0] = 0xE5; // 标记为删除
	SetEntry(eid, &e);
	return true;
}

__attribute__((regparm(2)))
bool mv(char *filename, char *target){
	Entry e;	
	bool diff = false;
	for(uint8_t i = 0;i < 11;++i){
		if (filename[i] != target[i]){
			diff = true;
			break;
		}
	}
	if (!diff)return false; // 没有更改名称
	ToFAT12Name(fat12name, filename);
	ToFAT12Name(targetname, target);
	uint16_t eid = FindEntry(fat12name, &e);
	if (eid == 0xFFFF)return false;
	memcpy(e.DIR_Name, targetname, 11);
	SetEntry(eid, &e);
	return true;
}

char cpbuf[512];
Entry te;
__attribute__((regparm(2)))
bool cp(char *filename, char *target){
	Entry e;	
	bool diff = false;
	for(uint8_t i = 0;i < 11;++i){
		if (filename[i] != target[i]){
			diff = true;
			break;
		}
	}
	if (!diff)return false; // 没有更改名称
	ToFAT12Name(fat12name, filename);
	ToFAT12Name(targetname, target);
	uint16_t eid = FindEntry(fat12name, &e);
	if (eid == 0xFFFF)return false;
	uint16_t cl = e.DIR_FstClus;
	uint16_t nid = FindEntry(targetname, &te);
	uint16_t ec = FindEmptyClus();
	if (ec == 0xFFFF)return false;
	e.DIR_FstClus = ec;
	SetClus(ec, 0xFFF);
	memcpy(e.DIR_Name, targetname, 11);

	if (nid != 0xFFFF){
		//删除旧数据
		//为了存储旧数据，暂时不实现
	}else{
		nid = FindEmptyEntry();
		if (nid == 0xFFFF)return false;
	}

	bool first = true;

	uint16_t wcl = ec;
	while(cl < 0xFF8){
		if (!first)wcl = GetNextClus(wcl);
		if (wcl == 0xFFFF)return false;
		first = false;
		//拷贝扇区
		ReadFloppy(33 + (cl - 2), 1, cpbuf);
		WriteFloppy(33 + (wcl - 2), 1, cpbuf);
		cl = GetNextFat(cl);
	}

	SetEntryTime(&e);
	SetEntry(nid, &e);
	return true;
}

#endif
