#include "include/io.h"
#include "include/disk.h"

char buf[512];
Entry e;

const char Months[12][4] = {"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};

__attribute__((regparm(1)))
void PrintDate(uint16_t date){
	//Month
	uint8_t month = (date >> 5) & 0xF;
	PrintStr(Months[month - 1]);
	PrintChar('.');
	uint8_t day = (date) & 0x1F;
	PrintNum(day);
	PrintChar(' ');
	uint16_t year = uint16_t((date >> 9) & 0x7F) + 1980;
	PrintNum(year);
}

__attribute__((regparm(1)))
void Print2Num(int num){
	PrintChar(num / 10 + '0');
	PrintChar(num % 10 + '0');
}

__attribute__((regparm(2)))
void PrintTime(uint16_t t,uint8_t offset){
	uint8_t second = (t & 0x1F) * 2;
	uint8_t minute = ((t>>5) & 0x3F);
	uint8_t hour = (((t>>11) & 0x1F) + offset)%24;
	Print2Num(hour);
	PrintChar(':');
	Print2Num(minute);
	PrintChar(':');
	Print2Num(second);
}

int main(){
	
	bool first = true;
	for (int i = 19;i < 19 + 14;++i){
		ReadFloppy(i,1,buf);
		for (int j = 0;j < 512/32;++j){
			for (int k = 0;k < 32;++k){
				*(((char*)&e) + k) = buf[k + j*32];
			}
			if (e.DIR_Name[10] == 0)continue;
			if (first){
				first = false;
				PrintStr("Filename    Size  Date       Time(UTC+8)",LBLUE);
				PrintStr(NEWLINE);
			}
			//Print Name
			int count = 0;
			for (int i = 0;i < 8;++i){
				if(e.DIR_Name[i] && e.DIR_Name[i] != ' '){
					PrintChar(e.DIR_Name[i]);
					++count;
				}
				else break;
			}
			PrintChar('.');
			for (int i = 8;i < 11;++i){
				if(e.DIR_Name[i] && e.DIR_Name[i] != ' '){
					PrintChar(e.DIR_Name[i]);
					++count;
				}
				else break;
			}
			for (int i = count;i < 11;++i)PrintChar(' ');
			count = PrintNum(e.DIR_FileSize);
			for(int i = count;i < 6;++i)PrintChar(' ');
			PrintDate(e.LAST_WrtDate);
			PrintChar(' ');
			//使用东八区时间
			PrintTime(e.LAST_WrtTime, 8);
			PrintStr(NEWLINE);
		}
	}
	
	return 0;
}
