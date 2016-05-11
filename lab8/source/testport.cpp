#include "port.h"

char GoodJob[32] = "GoodJob!";
int main(){
	WritePort(TALK_PORT,GoodJob,32);
	SetPortMsgV(TALK_PORT,1);
	return 0;
}
