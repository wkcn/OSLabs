#include "io.h"
#include "thread.h"

char str[80]="129djwqhdsajd128dw9i39ie93i8494urjoiew98kdkd";
int LetterNr=0;

__attribute__((regparm(1)))
void* Count(void *){
	for (int i = 0;str[i];++i){
		if (str[i] >= 'a' && str[i] <= 'z')LetterNr++;
	}
	return (void*)LetterNr;
}

int main(){
	thread_t tid;
	thread_create(&tid, Count, 0);
	thread_join(tid, 0);
	PrintStr("#Alpha of Str: ");
	PrintNum(LetterNr);
	PrintStr(NEWLINE);
	return 0;
}
