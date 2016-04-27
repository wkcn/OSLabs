#include "include/task.h"
#include "include/io.h"

char str[64] = "WhAlE aNd MoBuLA";
int main(){
	PrintStr(str,LBLUE);
	PrintStr(NEWLINE);
	if (fork()){
		//大写
		asm volatile(
				"mov dx,cs;"
				"int 0x24;"
				:
				:"a"(0x0100),"b"(str)
				:"dx"
				);
		PrintStr(str,RED);
	}else{
		//小写
		asm volatile(
				"mov dx,cs;"
				"int 0x24;"
				:
				:"a"(0x0000),"b"(str)
				:"dx"
				);
		PrintStr(str,LGREEN);
	}
	PrintStr(NEWLINE);
	return 0;
}
