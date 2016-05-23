#include <fstream>
#include <string>
using namespace std;
//320 x 240
//   / 16
//20 x 15
int main(){
	ofstream fout("map.asm");
	for (int r = 0;r < 15;++r){
		fout << "db ";
		for (int c = 0;c < 20;++c){
			if (c != 0)fout << ", ";
			if (c % 2 == 0){
				fout << 0;
			}else{
				fout << 1;
			}
		}
		fout << endl;
	}
	return 0;
}
