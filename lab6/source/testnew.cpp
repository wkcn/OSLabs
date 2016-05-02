#include "io.h"
#include "memory.h"

int main(){
	mem_init();
	int *a = new int;
	if (a != mem_null){
		*a = 10;
		PrintNum((uint16_t)(unsigned long)a,RED);
		PrintNum(*a);
		delete a;
		a = new int;
		*a = 30;
		PrintNum((uint16_t)(unsigned long)a,GREEN);
		PrintNum(*a);
		PrintStr(NEWLINE);
		delete a;
		a = new int[100];
		if (a == mem_null){
			PrintStr("Lack of Memory!");
			return 0;
		}
		a[0] = 1;
		a[1] = 1;
		for (int i = 2;i < 20;++i){
			a[i] = a[i-1] + a[i-2];
		}
		for (int i = 0;i < 20;++i){
			PrintNum(a[i]);
			PrintChar(',');
		}
	}else{
		PrintStr("Lack of Memory!");
	}
	PrintStr(NEWLINE);
	return 0;
}
