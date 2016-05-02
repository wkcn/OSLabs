#include "io.h"
#include "disk.h"

FAT12Header header;
int main(){

	ReadFloppy(0,1,&header);

	PrintStr("DrvNum db ");
	PrintNum(header.BS_DrvNum);
	PrintStr(NEWLINE);

	PrintStr("Reserved1 db ");
	PrintNum(header.BS_Reserved1);
	PrintStr(NEWLINE);

	PrintStr("BootSig db 0x");
	PrintHex(header.BS_BootSig);
	PrintStr(NEWLINE);

	PrintStr("VolID db ");
	PrintNum(header.BS_VolID);
	PrintStr(NEWLINE);

	PrintStr("VolLab db \"");
	PrintStrN(header.BS_VolLab,11);
	PrintChar('\"');
	PrintStr(NEWLINE);

	PrintStr("FileSysType db \"");
	PrintStrN(header.BS_FileSysType,8);
	PrintChar('\"');
	PrintStr(NEWLINE);
	return 0;
}
