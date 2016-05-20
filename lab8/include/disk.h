#ifndef _DISK_H_
#define _DISK_H_


#include "defines.h"
#include "io.h"

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
void ReadFloppy(uint16_t sectorID, uint8_t sectorNum, void *data){
	const uint16_t SecPerTrk = 18;
	//const uint16_t BytsPerSec = 512;
	uint8_t y = sectorID / SecPerTrk;
	uint8_t z = sectorID % SecPerTrk;
	uint8_t cl = z + 1; // 起始扇区号
	uint8_t ch = y >> 1; // 柱面号
	uint8_t dh = y & 1; // 磁头号
	uint8_t dl = 0; // 驱动器号, 0表示软盘A
	uint8_t ah = 2; // 功能号
	uint8_t al = sectorNum; // 读扇区数
	asm volatile(
			"push es;"
			"push ax;"
			"mov ax, ds;"
			"mov es, ax;"
			"pop ax;"
			"int 0x13;"
			"pop es;"
			:
			:"a"((ah<<8)|al),"b"(data),"c"((ch<<8)|cl),"d"((dh<<8)|dl)	
			);
}

__attribute__((regparm(3)))
void WriteFloppy(uint16_t sectorID, uint8_t sectorNum, char *data){
	const uint16_t SecPerTrk = 18;
	//const uint16_t BytsPerSec = 512;
	uint8_t y = sectorID / SecPerTrk;
	uint8_t z = sectorID % SecPerTrk;
	uint8_t cl = z + 1; // 起始扇区号
	uint8_t ch = y >> 1; // 柱面号
	uint8_t dh = y & 1; // 磁头号
	uint8_t dl = 0; // 驱动器号, 0表示软盘A
	uint8_t ah = 3; // 功能号
	uint8_t al = sectorNum; // 读扇区数
	asm volatile(
			"push es;"
			"push ax;"
			"mov ax, ds;"
			"mov es, ax;"
			"pop ax;"
			"int 0x13;"
			"pop es;"
			:
			:"a"((ah<<8)|al),"b"(data),"c"((ch<<8)|cl),"d"((dh<<8)|dl)	
			);

}

char febuf[512];
__attribute__((regparm(2)))
uint16_t FindEntry(char *filename, Entry *e){
	uint16_t id = 0;
	for (int i = 19;i < 19 + 14;++i){
		ReadFloppy(i,1,febuf);
		for (int j = 0;j < 512/32;++j){
			memcpy(e,febuf + j * 32,32);
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

char feebuf[512];
uint16_t FindEmptyEntry(){
	//暂时不使用E5标记文件的扇区
	uint16_t id = 0;
	for (int i = 19;i < 19 + 14;++i){
		ReadFloppy(i,1,feebuf);
		for (int j = 0;j < 512/32;++j){
			if (feebuf[j * 32] == 0){
				return id;
			}
			++id;
		}
	}
	return 0xFFFF;
}

char sebuf[512];
__attribute__((regparm(2)))
void SetEntry(uint16_t id, Entry *e){
	uint16_t q = id / (512 / 32) + 19;
	uint16_t t = id % (512 / 32);
	ReadFloppy(q, 1, sebuf);
	//*(Entry*)(sebuf + 32 * t) = *e;
	memcpy(sebuf + 32 * t, e, 32);
	WriteFloppy(q, 1, sebuf);
}

char gnfbuf[1024]; // 全局变量
__attribute__((regparm(1)))
uint16_t GetNextFat(uint16_t u){
	//get fat
	int t = u * 3 / 2;
	int p = t / 512;
	int o = t % 512;
	ReadFloppy(1 + p,2,gnfbuf);	
	//注意位扩展:-(
	uint16_t w = *(uint16_t*)(gnfbuf + o);
	if (u % 2 == 0){
		w &= 0xFFF;
	}else{
		w = (w >> 4) & 0xFFF;
	}
	return w;
}

char fecbuf[512 * 3];
uint16_t FindEmptyClus(){
	for (uint16_t q = 0;q < 9 / 3;++q){
		ReadFloppy(1 + 3 * q, 3, fecbuf);
		uint16_t o = 6;//start * 1.5; 
		uint8_t y = 1;
		//跳过FAT的项0, 项1.
		for (uint16_t u = 4;u < 512 * 2;++u){
			uint16_t w = *(uint16_t*)(fecbuf + o);
			if (u % 2 == 0){
				w &= 0xFFF;
			}else{
				w = (w >> 4) & 0xFFF;
			}
			if (w == 0){
				return u;
			}
			o += ((y)?1:2);
			if (y == 0)y = 1;
			else y = 0;
		}
	}
	return 0xFFFF;
}

char scbuf[512 * 3];
__attribute__((regparm(2)))
void SetClus(uint16_t id, uint16_t value){
	uint16_t q = id / 1024;
	uint16_t z = id % 1024;
	ReadFloppy(1 + 3 * q, 3, scbuf);
	uint16_t o = z * 3 / 2;
	uint16_t w = *(uint16_t*)(scbuf + o);
	value &= 0xFFF;
	if  (z % 2 == 0){
		//low
		w = (w & 0xF000) | value;
	}else{
		//high
		w = (w & 0x000F) | (value << 4);
	}
	*(uint16_t*)(scbuf + o) = w;
	WriteFloppy(1 + 3 * q, 3, scbuf); //FAT1
	WriteFloppy(10 + 3 * q, 3, scbuf); //FAT2
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
	char fat12name[11];
	Entry e;
	uint16_t _g,_p;
};
__attribute__((regparm(1)))
uint16_t tellg(File *f){
	return f->_g;
}
__attribute__((regparm(1)))
uint16_t tellp(File *f){
	return f->_p;
}


__attribute__((regparm(2)))
void ToFAT12Name(char *fat12name, char *filename){
	//转为FAT12的文件名
	for(uint16_t w = 0;w < 11;++w)fat12name[w] = ' ';
	uint16_t i = 0;
	uint16_t q = 0;
	char c;
	for (;i < 8 && filename[i] != '.';++i){
		c = filename[i];
		if (c >= 'a' && c <= 'z')c = c - 'a' + 'A';
		fat12name[q++] = c;
	}
	q = 8;
	if (filename[i] == '.'){
		for (uint16_t w = 1;w <= 3;++w){
			c = filename[i + w];
			if (c >= 'a' && c <= 'z')c = c - 'a' + 'A';
			fat12name[q++] = c;
		}
	}
}

__attribute__((regparm(1)))
void SetEntryTime(Entry *e){
	uint16_t chcl, dhdl;
	uint16_t tch,tcl,tdh,tdl;
	asm volatile(
			"int 0x1a;"
			:"=c"(chcl),"=d"(dhdl)
			:"a"(0x0200)
			);
	//得到的是BCD码
	tch = chcl >> 8;
	tcl = chcl & 0xFF;
	tdh = dhdl >> 8;
	tdl = dhdl & 0xFF;
	tdh = BCD2HEX(tdh);
	tdl = BCD2HEX(tdl);
	tch = BCD2HEX(tch);
	tcl = BCD2HEX(tcl);
	e->DIR_WrtTime = (tch << 11) | (tcl << 5) | (tdh >> 1);
	e->LAST_WrtTime = e->DIR_WrtTime;
	asm volatile(
			"int 0x1a;"
			:"=c"(chcl), "=d"(dhdl)
			:"a"(0x0400)
			);

	tdh = dhdl >> 8;
	tdl = dhdl & 0xFF;
	tdh = BCD2HEX(tdh);
	tdl = BCD2HEX(tdl);
	e->DIR_WrtDate = ((BCD2HEX(chcl) - 1980) << 9) | (tdh << 5) | tdl; 
	e->LAST_WrtDate = e->DIR_WrtDate;
	
}

__attribute__((regparm(2)))
bool open(File *f, char *filename){
	memcpy(f->filename, filename, 11);
	ToFAT12Name(f->fat12name, filename);
	f->_g = f->_p = 0;
	return FindEntry(f->fat12name, &f->e) != 0xFFFF;
}
__attribute__((regparm(1)))
uint16_t size(File *f){
	return f->e.DIR_FileSize;
}
__attribute__((regparm(2)))
void seekg(File *f,uint16_t g){
	f->_g = g;
}
__attribute__((regparm(2)))
void seekp(File *f,uint16_t p){
	f->_p = p;
}
char rdbuf[512];
__attribute__((regparm(3)))
uint16_t read(File *f, char *data, uint16_t size){
	if (f->fat12name[0] == ' ' || !f->fat12name[0])return false;
	if(FindEntry(f->fat12name, &f->e) == 0xFFFF)return false;
	// _g
	uint16_t s = f->_g / 512; // 第几块
	uint16_t o = f->_g % 512; // 块中偏移字节
	//跳转到第s块
	uint16_t u = f->e.DIR_FstClus;
	for (uint16_t sc = 0;sc < s;++sc){
		u = GetNextFat(u);
		if (u >= 0xFF8)return 0;
	}
	ReadFloppy((33 + (u - 2)),1,rdbuf);
	// 开始写入到data
	u = GetNextFat(u);
	uint16_t blockSize = 512;
	uint16_t lastBlockSize = 512;
	uint16_t b = f->e.DIR_FileSize % 512;
	if (b > 0)lastBlockSize = b;
	if (u >= 0xFF8)blockSize = lastBlockSize;
	for (uint16_t i = 0;i < size;++i){
		if (o >= blockSize){
			//当前扇区已经读完
			if (u >= 0xFF8){
				f->_g += i;
				return i;
			}
			ReadFloppy((33 + (u - 2)),1,rdbuf);
			o = 0;
			u = GetNextFat(u);
			if (u >= 0xFF8)blockSize = lastBlockSize;
		}
		data[i] = rdbuf[o++];
	}
	//更新_g
	f->_g += size;
	return size;
}


char wtbuf[512];
__attribute__((regparm(3)))
bool write(File *f, char *data, uint16_t size){
	if (f->fat12name[0] == ' ' || !f->fat12name[0])return false;
	uint16_t eid;
	if((eid = FindEntry(f->fat12name, &f->e)) == 0xFFFF){
		//建立Entry
		memcpy(f->e.DIR_Name, f->fat12name, 11);
		f->e.DIR_Attr = 0x20; // 档案
		uint16_t ec = FindEmptyClus();
		if (ec == 0xFFFF)return false;
		f->e.DIR_FstClus = ec;
		SetClus(ec, 0x0FFF);
		f->e.DIR_FileSize = 0;
		eid = FindEmptyEntry();
	}
	// _p
	uint16_t s = f->_p / 512; // 第几块
	uint16_t o = f->_p % 512; // 块中偏移字节
	//查找当前块是否足够, 若不足够则扩展
	uint16_t cl = f->e.DIR_FstClus;
	for (uint16_t q = 0;q < s;++q){
		cl = GetNextClus(cl);
		if (cl == 0xFFFF)return false;
		//cl绝对是可用的
	}
	//当前cl可用
	//写入数据
	if (size < 512 || o > 0)ReadFloppy(33 + cl - 2,1,wtbuf); //保留前面或后面的数据
	for (uint16_t i = 0;i < size;++i){
		if (o >= 512){
			WriteFloppy(33 + cl - 2,1,wtbuf); 
			cl = GetNextClus(cl);
			if (cl == 0xFFFF)return false;
			if (size - i < 512){
				ReadFloppy(33 + cl - 2,1,wtbuf);
			}
			o = 0;
		}
		wtbuf[o++] = data[i];
	}
	WriteFloppy(33 + cl - 2,1,wtbuf); 
	if (f->_p + size > f->e.DIR_FileSize)
		f->e.DIR_FileSize = f->_p + size;
	f->_p += size;
	//更新Entry Date and Time
	SetEntryTime(&(f->e));
	SetEntry(eid, &f->e); //更新Entry
	return true;
}

__attribute__((regparm(3)))
uint16_t LoadFile(char *filename, uint16_t offset, uint16_t seg){
	char lfbuf[1024];
	Entry e;
	for (int i = 19;i < 19 + 14;++i){
		ReadFloppy(i,1,lfbuf);
		for (int j = 0;j < 512/32;++j){
			memcpy(&e,lfbuf + j * 32,32);
			bool can = true;
			for (int k = 0;k < 11;++k){
				if (filename[k] != e.DIR_Name[k]){
					can = false;
					break;
			 	}
			} 
			if (!can)continue;
			//FOUND_ENTRY
			uint16_t u = e.DIR_FstClus;
			while (!(u >= 0xFF8)){
				//int offset = 512 * 33 + (u - 2) * 512;
				int y = 33 + (u - 2);

				ReadFloppy(y,1,lfbuf);
				for (int w = 0;w < 512;++w){
					asm volatile(
							"push es;"
							"push si;"
							"mov es, ax;"
							"mov si, bx;"
							"mov es:[si], cl;"
							"pop si;"
							"pop es;"
							:
							:"a"(seg),"b"(offset),"c"(lfbuf[w])
							);
					offset += 1;
					if (offset == 0x0000){
						//0x0 0000
						seg += 0x1000;
					}
				}
				//memcpy(dest,buf,512); // 这里按512覆盖, 不考虑不满512的情况
				//dest += 512;

				//get fat
				int t = u * 3 / 2;
				int p = t / 512;
				int o = t % 512;
				ReadFloppy(1 + p,2,lfbuf);	
				//注意位扩展:-(
				uint16_t w = *(uint16_t*)(lfbuf + o);
				if (u % 2 == 0){
					w &= 0xFFF;
				}else{
					w = (w >> 4) & 0xFFF;
				}
				u = w;
			} 
			return e.DIR_FileSize;
		}
	}
	return 0;
}

__attribute__((regparm(1)))
uint16_t GetFileSize(char *filename){
	char fsbuf[512];
	Entry e;
	for (int i = 19;i < 19 + 14;++i){
		ReadFloppy(i,1,fsbuf);
		for (int j = 0;j < 512/32;++j){
			memcpy(&e,fsbuf + j * 32,32);
			bool can = true;
			for (int k = 0;k < 11;++k){
				if (filename[k] != e.DIR_Name[k]){
					can = false;
					break;
			 	}
			} 
			if (!can)continue;
			//FOUND_ENTRY
			return e.DIR_FileSize;
		}
	}
	return 0;
}
#endif
