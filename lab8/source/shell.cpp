#include "prog.h"
#include "port.h"
ReadyProg readyprog;
char filename[12] = "LS      COM";
int main(){
	for(int i = 0;i < 12;++i)
		readyprog.filename[i] = filename[i];
	readyprog.allocateSize = 0;
	WritePort(READYPROG_PORT, &readyprog, sizeof(ReadyProg));
	SetPortMsgV(READYPROG_PORT,1);
	return 0;
}
