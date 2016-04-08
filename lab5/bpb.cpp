#include "include/io.h"
//#include "include/disk.h"

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


__attribute__((regparm(3)))
void ReadFloppy(uint16_t sectorID, uint8_t sectorNum, char *data){
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


FAT12Header header;
int main(){
	ReadFloppy(0,1,(char*)&header);

	PrintStr("jmpShort dw 0x");
	PrintHex((header.jmpShort >> 8) & 0xFF);
	PrintHex((header.jmpShort) & 0xFF);
	PrintStr(NEWLINE);
	PrintStr("nop");
	PrintStr(NEWLINE);
	PrintStr("OEMName db \"");
	PrintStrN(header.BS_OEMName,8);
	PrintChar('\"');
	PrintStr(NEWLINE);

	PrintStr("BytesPerSec dw ");
	PrintNum(header.BPB_BytesPerSec);
	PrintStr(NEWLINE);

	PrintStr("SecPerClus db ");
	PrintNum(header.BPB_SecPerClus);
	PrintStr(NEWLINE);

	PrintStr("ResvdSecCnt dw ");
	PrintNum(header.BPB_ResvdSecCnt);
	PrintStr(NEWLINE);

	PrintStr("NumFATs db ");
	PrintNum(header.BPB_NumFATs);
	PrintStr(NEWLINE);

	PrintStr("RootEntCnt dw ");
	PrintNum(header.BPB_RootEntCnt);
	PrintStr(NEWLINE);

	PrintStr("TotSec16 dw ");
	PrintNum(header.BPB_TotSec16);
	PrintStr(NEWLINE);

	PrintStr("Media db 0x");
	PrintHex(header.BPB_Media);
	PrintStr(NEWLINE);

	PrintStr("FatSz16 dw ");
	PrintNum(header.BPB_FATSz16);
	PrintStr(NEWLINE);

	PrintStr("SecPerTrk dw ");
	PrintNum(header.BPB_SecPerTrk);
	PrintStr(NEWLINE);

	PrintStr("NumHeads dw ");
	PrintNum(header.BPB_NumHeads);
	PrintStr(NEWLINE);

	PrintStr("HiddSec dd 0x");
	PrintNum(header.BPB_HiddSec);
	PrintStr(NEWLINE);

	PrintStr("TotSec32 dd 0x");
	PrintNum(header.BPB_TotSec32);
	PrintStr(NEWLINE);

	return 0;
}
