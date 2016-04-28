#include "include/io.h"
#include "include/thread.h"

const int M = 3;
const int K = 2;
const int N = 3;
const int NUM_THREADS = M * N;
int A[M][K]={{1,4},{2,5},{3,6}}; 
int B[K][N]={{8,7,6},{5,4,3}};
int C[M][N];

__attribute__((regparm(1)))
void* CalcOneElem(void* i){
	PrintStr("hello");
	PrintNum(*(uint8_t*)i);
	PrintStr(NEWLINE);
	return 0;
}

uint8_t i = 2;
uint8_t j = 4;
int main(){
	thread_t tid[NUM_THREADS];
	uint8_t q = thread_create(&tid[0],CalcOneElem,&i);
	PrintNum(q,RED);
	PrintStr("WW");
	uint8_t w = thread_create(&tid[1],CalcOneElem,&j);
	PrintNum(w,GREEN);
	PrintStr("END");
	//i = 2;
	//thread_create(&tid[1],CalcOneElem,&i);
	thread_join(tid[0],0);
	thread_join(tid[1],0);
	return 0;
}
