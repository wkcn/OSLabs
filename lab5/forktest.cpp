#include "include/io.h"
#include "include/task.h"

int main(){
	if (fork() == 0){
		for (int i = 0;i < 20;++i){
			PrintChar('.');
		}
	}else{
		for (int i = 0;i < 20;++i){
			PrintChar('o');
		}
	}
	PrintStr(NEWLINE);
	return 0;
}
