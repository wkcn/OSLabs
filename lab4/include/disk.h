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


void ls(){
	char buf[512];
	Entry e; // 效率需要
	for (int i = 19;i < 19 + 14;++i){
		ReadFloppy(i,1,buf);
		for (int j = 0;j < 512/32;++j){
			memcpy(&e,buf + j * 32,32);
			PrintStr(e.DIR_Name,11);
			if(e.DIR_Name[1] != 0)cout << endl;
 		}
 	}
} 
 
__attribute__((regparm(3)))
int LoadFile(char *filename, uint16_t offset, uint16_t seg){
	char buf[1024];
	Entry e;
	for (int i = 19;i < 19 + 14;++i){
		ReadFloppy(i,1,buf);
		for (int j = 0;j < 512/32;++j){
			memcpy(&e,buf + j * 32,32);
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

				ReadFloppy(y,1,buf);
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
							:"a"(seg),"b"(offset),"c"(buf[w])
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
				ReadFloppy(1 + p,2,buf);	
				//uint16_t w = ((buf[o+1]&0xFF) << 8) | (buf[o]&0xFF);
				//注意位扩展:-(
				uint16_t w = *(uint16_t*)(buf + o);
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

#endif
