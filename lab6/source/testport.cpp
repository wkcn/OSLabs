#include "port.h"

char GoodJob[32] = "GoodJob!";
int main(){
	WritePort(5,GoodJob,32);
	SetPortMsgV(5,1);
	return 0;
}
