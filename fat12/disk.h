#ifndef _DISK_H_
#define _DISK_H_

#include <fstream>
#include <iostream>
#include <stdint.h>
using namespace std;

typedef char db;
typedef uint16_t dw;
typedef uint32_t dd;
typedef uint64_t dq;

#pragma pack (1) // 按1字节对齐
struct FAT12Header{
	dw jmpShort;//BS_jmpBOOT 一个短跳转指令
	db nop;
	db BS_OEMName[8];	// 厂商名
	dw BPB_BytesPerSec; //每扇区字节数（Bytes/Sector）	0x200
	db BPB_SecPerClus;	//每簇扇区数（Sector/Cluster）	0x1
	dw BPB_ResvdSecCnt;	//Boot记录占用多少扇区	ox1
	db BPB_NumFATs;	//共有多少FAT表	0x2
	dw BPB_RootEntCnt;	//根目录区文件最大数	0xE0
	dw BPB_TotSec16;	//扇区总数	0xB40[2*80*18]
	db BPB_Media;	//介质描述符	0xF0
	dw BPB_FATSz16;	//每个FAT表所占扇区数	0x9
	dw BPB_SecPerTrk;	//每磁道扇区数（Sector/track）	0x12
	dw BPB_NumHeads;	//磁头数（面数）	0x2
	dd BPB_HiddSec;	//隐藏扇区数	0
	dd BPB_TotSec32;	//如果BPB_TotSec16=0,则由这里给出扇区数	0
	db BS_DrvNum;	//INT 13H的驱动器号	0
	db BS_Reserved1;	//保留，未使用	0
	db BS_BootSig;	//扩展引导标记(29h)	0x29
	dd BS_VolID;	//卷序列号	0
	db BS_VolLab[11];	//卷标 'wkcn'
	db BS_FileSysType[8];	//文件系统类型	'FAT12'
	db other[448];	//引导代码及其他数据	引导代码（剩余空间用0填充）
	dw _55aa;	//第510字节为0x55，第511字节为0xAA	0xAA55
};


#pragma pack (1) // 按1字节对齐
struct Entry{
	db DIR_Name[11];
	db DIR_Attr;
	db temp;
	db ratio;
	dw DIR_WrtTime;
	dw DIR_WrtDate;
	dw DIR_VISDate;
	dw FAT32_HIGH;
	dw LAST_WrtTime;
	dw LAST_WrtDate;
	dw DIR_FstClus;
	dd DIR_FileSize;
};

__attribute__((regparm(3)))
void ReadFloppy(uint16_t sectorID, uint8_t sectorNum, char *data){
	ifstream fin("disk.img", ios::binary);
	fin.seekg(sectorID * 512, ios::beg);
	fin.read(data, sectorNum * 512);
}

char buf[1024]; // 全局变量

__attribute__((regparm(2)))
bool FindEntry(char *filename, Entry *e){
	for (int i = 19;i < 19 + 14;++i){
		ReadFloppy(i,1,buf);
		for (int j = 0;j < 512/32;++j){
			memcpy(e,buf + j * 32,32);
			bool can = true;
			for (int k = 0;k < 11;++k){
				if (filename[k] != e->DIR_Name[k]){
					can = false;
					break;
			 	}
			} 
			if (!can)continue;
			//FOUND_ENTRY
			return true;
		}
	}
	return false;
}

__attribute__((regparm(1)))
uint16_t GetNextFat(uint16_t u){
	//get fat
	int t = u * 3 / 2;
	int p = t / 512;
	int o = t % 512;
	ReadFloppy(1 + p,2,buf);	
	//注意位扩展:-(
	uint16_t w = *(uint16_t*)(buf + o);
	if (u % 2 == 0){
		w &= 0xFFF;
	}else{
		w = (w >> 4) & 0xFFF;
	}
	return w;
}

struct File{
	char filename[11];
	Entry e;
	uint16_t _g,_p;
	bool eofed;
	__attribute__((regparm(1)))
	void open(const char *filename){
		memcpy(this->filename, filename, 11);
		_g = _p = 0;
	}
	uint16_t size(){
		return e.DIR_FileSize;
	}
	__attribute__((regparm(1)))
	void seekg(uint16_t g){
		_g = g;
	}
	__attribute__((regparm(1)))
	void seekp(uint16_t p){
		_p = p;
	}
	__attribute__((regparm(1)))
	bool read(char *data, uint16_t size){
		FindEntry(filename, &e);
		// _g
		uint16_t s = _g / 512; // 第几块
		uint16_t o = _g % 512; // 块中偏移字节
		//跳转到第s块
		uint16_t u = e.DIR_FstClus;
		for (uint16_t sc = 0;sc < s;++sc){
			u = GetNextFat(u);
			if (u >= 0xFF8)return false;
		}
		ReadFloppy((33 + (u - 2)),1,buf);
		// 开始写入到data
		for (uint16_t i = 0;i < size;++i){
			if (o >= 512){
				//当前扇区已经读完
				u = GetNextFat(u);
				if (u >= 0xFF8)return false;
				ReadFloppy((33 + (u - 2)),1,buf);
				o = 0;
			}
			data[i] = buf[o++];
		}
		//更新_g
		_g += size;
		return true;
	}
	__attribute__((regparm(1)))
	bool write(char *data, uint16_t size){
	}
};

#endif
