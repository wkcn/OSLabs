#include "include/io.h"
#include "include/disk.h"

FAT12Header header;
int main(){
	ReadFloppy(0,1,&header);
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

	PrintStr("HiddSec dd ");
	PrintNum(header.BPB_HiddSec);
	PrintStr(NEWLINE);

	PrintStr("TotSec32 dd ");
	PrintNum(header.BPB_TotSec32);
	PrintStr(NEWLINE);

	return 0;
}
