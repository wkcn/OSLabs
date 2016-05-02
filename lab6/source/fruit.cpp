#include "io.h"
#include "thread.h"
#include "sem.h"

uint16_t count = 0;
uint8_t sid;
uint16_t fid[2];
bool useSem = true;
uint16_t total = 50;

__attribute__((regparm(1)))
void * GiveFruit(void *p){
	for (int i = 0;i < total / 2;++i){
		if(useSem)semWait(sid);
		PrintNum(*(uint16_t*)(p), LBLUE);// 进入
		uint16_t old;
		old = count;
		for (uint16_t w=0;w<0xFFFF;++w){
			for (uint16_t ww=0;ww<0xFF;++ww){
				for (uint16_t www=0;www<0x1;++www){
				}
			}
		}
		count = old + 1;
		PrintNum(count,RED); // 数字
		PrintNum(*(uint16_t*)(p),LGREEN); // 出来
		if(useSem)semSignal(sid);
	}
	return 0;
}

int main(){
	thread_t tid[2];
	sid = semCreate(1);
	fid[0] = 0;
	fid[1] = 1;

	thread_create(&tid[0],GiveFruit,&fid[0]);
	thread_create(&tid[1],GiveFruit,&fid[1]);
	thread_join(tid[0],0);
	thread_join(tid[1],0);
	PrintStr("I have ");
	PrintNum(count);
	PrintStr(" fruits, I should have ");
	PrintNum(total);
	PrintStr(NEWLINE);

	PrintStr("I don't use sem!\r\n",LGREEN);
	count = 0;
	useSem = 0;
	thread_create(&tid[0],GiveFruit,&fid[0]);
	thread_create(&tid[1],GiveFruit,&fid[1]);
	thread_join(tid[0],0);
	thread_join(tid[1],0);
	PrintStr("I have ");
	PrintNum(count);
	PrintStr(" fruits, I should have ");
	PrintNum(total);
	PrintStr(NEWLINE);

	return 0;
}
