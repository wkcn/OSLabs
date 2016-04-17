#include "include/io.h"
#include "include/task.h"

ostream cout;
int main(){  
	int i=0;  
	for(i=0;i<3;i++){  
		uint8_t fpid=fork();  
		if(fpid==0)  
			PrintStr("son", YELLOW);
		else  
			PrintStr("FATHER",LGREEN);
		PrintStr(NEWLINE);
	}  
	return 0;  

}  
