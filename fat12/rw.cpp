#include "disk.h"

char data[2048];
int main(){
	File file;
	file.open("DISK    H  ");
	int size = file.size();
	char *data = new char[size];
	cout << "FileSize" << size << endl;
	file.read(data, size);
	for (int i = 0;i < size;++i){
		cout << data[i];
	}
	return 0;
}