#include "io.h"
#include "fork.h"

int main(){
	if (fork() == 0){
		for (int i = 0;i < 20;++i){
			PrintChar('.');
		}
	}else{
		if (fork() == 0){
			for (int i = 0;i < 20;++i){
				PrintChar('o');
			}
		}else{
			for (int i = 0;i < 20;++i){
				PrintChar('x');
			}
		}
	}
	PrintStr(NEWLINE);
	return 0;
}
