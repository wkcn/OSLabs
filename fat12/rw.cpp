#include "disk.h"

char data[204800];
int si = 0;
int main(){
	File file;
	file.open("WACK    EXE");
	if (1){

		ifstream fin("test");
		char c;
		c = fin.get();
		while(!fin.eof()){
			data[si++] = c;
			c = fin.get();
		}
		file.write(data,si);
	}else{
		file.read(data, file.size());
		for (int i = 0;i < file.size();++i){
			cout <<data[i];
		}
	}
	return 0;
}
