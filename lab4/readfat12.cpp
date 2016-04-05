#include <iostream>
#include <fstream>
#include <cstring>
using namespace std;

typedef char db;
typedef short dw;
typedef int dd;

char FileName[11 + 1] = "HELLO   COM";

#pragma pack (1) // 按1字节对齐
struct Header{
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

#pragma pack (1) // 按1字节对齐
struct FAT{
	db data[3];
};

void PrintChar(char c){
	cout << c;
}

fstream fin("build/disk.img",ios::binary | ios::in);
void PrintHex(char num){
	const char ch[16] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
	PrintChar(ch[(num>>4)&0xF]);
	PrintChar(ch[(num)&0xF]);
	cout << endl;
}
int getFAT(int i){
	//用户数据区的第一个簇的序号是002
	fin.seekg(512 * 1 + i * 3 / 2);
	int f;
	fin.read((char*)&f,2);
	cout << "hex" << endl;
	PrintHex(((f&0xFF00) >> 8));
	PrintHex(f&0xFF);

	// 00 00 00 00 00 00 00
	if (i % 2 == 1){
		//高12
		f = (f >> 4) & 0xFFF;
	}else{
		//低12
		f &= 0xFFF;
	}
	cout << "fat   " << f << endl;
	return f;
}

bool matchSTR(char *astr,char *bstr,int len){
	for (int i = 0;i < len;++i){
		if (astr[i] != bstr[i])return false;
	}
	return true;
}

bool FindEntry(Entry &e){
	for (int j = 0; j < 14;++j){
		fin.seekg((19+j) * 512, ios::beg);
		for (int k = 0;k < 512 / 32;++k){
			fin.read((char*)&e, 32);
			cout << "Te:" << e.DIR_Name << endl;
			if (matchSTR(e.DIR_Name, FileName,11)){
				return true;
			}
		}
	}
	return false;
}

int main(){
	cout << "See you" << endl;
	Header header;
	fin.read((char*)&header, 512);
	cout << header.BS_OEMName << endl;
	Entry e;
	bool can = FindEntry(e); 
	cout <<"Find" << can << endl;
	cout << e.DIR_FileSize << endl;
	int u = e.DIR_FstClus;
	//ofstream fout(FileName, ios::binary);
	char buf[512];
	while(!(u >= 0xFF8)){
		//簇
		int offset = 512 * 33 + (u - 2) * 512;
		//fin.seekg(offset,ios::beg);
		//fin.read(buf,512);
		cout << u<< endl;
		u = getFAT(u);
		if (u >= 0xFF8 && e.DIR_FileSize % 512 != 0){
			//fout.write(buf,e.DIR_FileSize % 512);
		}else{
			//fout.write(buf,512);
		}
	}
	return 0;
}
