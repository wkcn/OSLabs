#ifndef _DISK_H_
#define _DISK_H_

#include <fstream>
#include <iostream>
#include <stdint.h>
#include <cstring>
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
	fin.close();
}

__attribute__((regparm(3)))
void WriteFloppy(uint16_t sectorID, uint8_t sectorNum, char *data){
	//ofstream fout("disk.imgw", ios::binary|ios::app);
	//fout.seekp(sectorID * 512, ios::beg);
	//fout.write(data, sectorNum * 512);
	//fout.close();
	ifstream fin("disk.img",ios::binary);
	char buf[1440 * 1024];
	fin.read(buf,1440*1024);
	fin.close();
	for (int i = 0;i < sectorNum * 512;++i){
		buf[sectorID * 512 + i] = data[i];
	}
	ofstream fout("disk.img",ios::binary);
	fout.write(buf,1440*1024);
	fout.close();
}



__attribute__((regparm(2)))
uint16_t FindEntry(char *filename, Entry *e){
	char buf[512];
	uint16_t id = 0;
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
			if (!can){
				++id;
				continue;
			}
			//FOUND_ENTRY
			return id;
		}
	}
	return 0xFFFF;
}

uint16_t FindEmptyEntry(){
	char buf[512];
	uint16_t id = 0;
	for (int i = 19;i < 19 + 14;++i){
		ReadFloppy(i,1,buf);
		for (int j = 0;j < 512/32;++j){
			if (buf[j * 32] == 0){
				return id;
			}
			++id;
		}
	}
	return 0xFFFF;
}

__attribute__((regparm(2)))
void SetEntry(uint16_t id, Entry *e){
	uint16_t q = id / (512 / 32) + 19;
	uint16_t t = id % (512 / 32);
	char buf[512];
	ReadFloppy(q, 1, buf);
	*(Entry*)(buf + 32 * t) = *e;
	WriteFloppy(q, 1, buf);
}

__attribute__((regparm(1)))
uint16_t GetNextFat(uint16_t u){
	char buf[1024]; // 全局变量
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

uint16_t FindEmptyClus(){
	char buf[512 * 3];
	uint16_t id = 0;
	for (uint16_t q = 0;q < 9 / 3;++q){
		ReadFloppy(1 + 3 * q, 3, buf);
		uint16_t o = 0; 
		bool y = 1;
		for (uint16_t u = 0;u < 512 * 2;++u){
			uint16_t w = *(uint16_t*)(buf + o);
			if (u % 2 == 0){
				w &= 0xFFF;
			}else{
				w = (w >> 4) & 0xFFF;
			}
			if (w == 0){
				return id;
			}
			o += ((y)?1:2);
			y = !y;
			++id;
		}
	}
	return 0xFFFF;
}

__attribute__((regparm(2)))
void SetClus(uint16_t id, uint16_t value){
	char buf[512 * 3];
	uint16_t q = id / 1024;
	uint16_t z = id % 1024;
	ReadFloppy(1 + 3 * q, 3, buf);
	uint16_t o = z * 3 / 2;
	uint16_t w = *(uint16_t*)(buf + o);
	value &= 0xFFF;
	if (z % 2 == 0){
		//low
		w = (w & 0xF000) | value;
	}else{
		//high
		w = (w & 0x000F) | (value << 4);
	}
	*(uint16_t*)(buf + o) = w;
	WriteFloppy(1 + 3 * q, 3, buf); //FAT1
	WriteFloppy(10 + 3 * q, 3, buf); //FAT2
}

__attribute__((regparm(1)))
uint16_t GetNextClus(uint16_t cl){
	uint16_t nf = GetNextFat(cl);
	if (nf >= 0xFF8){
		uint16_t ec = FindEmptyClus();
		if (ec == 0xFFFF)return 0xFFFF;
		SetClus(cl,ec);
		SetClus(ec,0x0FFF);
		return ec;
	}
	return nf;	
}

struct File{
	char filename[11];
	Entry e;
	uint16_t _g,_p;
	bool eofed;
	uint16_t tellg(){
		return _g;
	}
	uint16_t tellp(){
		return _p;
	}
	__attribute__((regparm(1)))
	void open(const char *filename){
		memcpy(this->filename, filename, 11);
		FindEntry(this->filename, &e);
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
		if(FindEntry(filename, &e) == 0xFFFF)return false;
		// _g
		uint16_t s = _g / 512; // 第几块
		uint16_t o = _g % 512; // 块中偏移字节
		//跳转到第s块
		uint16_t u = e.DIR_FstClus;
		for (uint16_t sc = 0;sc < s;++sc){
			u = GetNextFat(u);
			if (u >= 0xFF8)return false;
		}
		char buf[512];
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
	__attribute__((regparm(2)))
	bool write(char *data, uint16_t size){
		uint16_t eid;
		if((eid = FindEntry(filename, &e)) == 0xFFFF){
			//建立Entry
			/*
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
			*/
			memcpy(e.DIR_Name, filename, 11);
			e.DIR_Attr = 0x20; // 档案
			uint16_t ec = FindEmptyClus();
			if (ec == 0xFFFF)return false;
			e.DIR_FstClus = ec;
			SetClus(ec, 0x0FFF);
			e.DIR_FileSize = 0;
			eid = FindEmptyEntry();
		}
		// _p
		uint16_t s = _p / 512; // 第几块
		uint16_t o = _p % 512; // 块中偏移字节
		//查找当前块是否足够, 若不足够则扩展
		uint16_t cl = e.DIR_FstClus;
		for (uint16_t q = 0;q < s;++q){
			cl = GetNextClus(cl);
			if (cl == 0xFFFF)return false;
			//cl绝对是可用的
		}
		//当前cl可用
		//写入数据
		char buf[512];
		if (size < 512 || o > 0)ReadFloppy(33 + cl - 2,1,buf); //保留前面或后面的数据
		for (uint16_t i = 0;i < size;++i){
			if (o >= 512){
				WriteFloppy(33 + cl - 2,1,buf); 
				cl = GetNextClus(cl);
				if (cl == 0xFFFF)return false;
				if (size - i < 512){
					ReadFloppy(33 + cl - 2,1,buf);
				}
				o = 0;
			}
			buf[o++] = data[i];
		}
		WriteFloppy(33 + cl - 2,1,buf); 
		if (_p + size > e.DIR_FileSize)
			e.DIR_FileSize = _p + size;
		_p += size;
		SetEntry(eid, &e); //更新Entry
		return true;
	}
};

#endif
