#include <fstream>
#include <iostream>
#include <string>
using namespace std;
string head = "#define RELEASE_TIMES ";
string filename = "include/version.h";
int main(){
	fstream fin(filename.c_str());
	fin.seekg(head.size());
	int v;
	fin >> v;
	++v;
	cout << head << v << endl;
	fin.seekp(0,ios::beg);
	fin << head << v << endl;
	return 0;
}
