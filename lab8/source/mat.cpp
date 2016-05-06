#include "io.h"
#include "thread.h"

const int M = 3;
const int K = 2;
const int N = 3;
const int NUM_THREADS = M * N;
int A[M][K]={{1,4},{2,5},{3,6}}; 
int B[K][N]={{8,7,6},{5,4,3}};
int C[M][N];

struct RC{
	int r,c;
};

RC data[NUM_THREADS];

__attribute__((regparm(1)))
void* CalcOneElem(void* i){
	RC rc = *(RC*)i;
	int r = rc.r;
	int c = rc.c;
	C[r][c] = 0;
	for (int i = 0;i < K;++i){
		C[r][c] += A[r][i] * B[i][c];
	}
	return 0;
}


__attribute__((regparm(3)))
void PrintMat(int *a, int r, int c){
	for (int _r = 0;_r < r;++_r){
		PrintStr("      ");
		for (int _c = 0;_c < c;++_c){
			PrintNum(a[_r * c + _c]);
			PrintChar(' ');
		}
		PrintStr(NEWLINE);
	}
}

int main(){
	thread_t tid[NUM_THREADS];
	for (int i = 0;i < M;++i){
		for (int j = 0;j < N;++j){
			int id = i * N + j;
			data[id].r = i;
			data[id].c = j;
			thread_create(&tid[id],CalcOneElem,&data[id]);
		}
	}
	//等待所有线程结束
	for (int i = 0;i < NUM_THREADS;++i){
		thread_join(tid[i],0);
	}	
	PrintMat((int*)A,M,K);
	PrintStr("        X\r\n",GREEN);
	PrintMat((int*)B,K,N);
	PrintStr("The result is: \r\n");
	PrintMat((int*)C,M,N);
	PrintStr(NEWLINE);
	return 0;
}
