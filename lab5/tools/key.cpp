#include <fstream>
#include <iostream>
#include <string>
#include <vector>
using namespace std;

struct K{
	string name;
	string smm[4];
	string asc[4];
};

int main(){
	ifstream fin("key.txt");
	string name;
	string temp;

	vector<K> ks;
	do{
		K k;
		fin >> name;
		if(fin.eof())break;
		for (int i = 0;i < name.size();++i){
			char c = name[i];
			if (c >= 'a' && c <= 'z')c = c - 'a' +'A';
			k.name += c;
		}
		for (int i = 0;i < 4;++i){
			fin >> k.smm[i];
			fin >> k.asc[i];
		}
		ks.push_back(k);
	}while(!fin.eof());
	ofstream fout("keyboard.h");
	fout << "#ifndef _KEY_BOARD_H_" << endl;
	fout << "#define _KEY_BOARD_H_" << endl << endl;
	int o = 0;
	for (size_t i = 0;i < ks.size();++i){
		K &k = ks[i];
		string name = "KEY_" + k.name;
		fout << "#define    " << name << "    " << "0x" << k.smm[o] << k.asc[o] << endl;
	}
	o = 1;
	for (size_t i = 0;i < ks.size();++i){
		K &k = ks[i];
		string name = "KEY_SHIFT_" + k.name;
		fout << "#define    " << name << "    " << "0x" << k.smm[o] << k.asc[o] << endl;
	}
	o = 2;
	for (size_t i = 0;i < ks.size();++i){
		K &k = ks[i];
		string name = "KEY_CTRL_" + k.name;
		fout << "#define    " << name << "    " << "0x" << k.smm[o] << k.asc[o] << endl;
	}
	o = 3;
	for (size_t i = 0;i < ks.size();++i){
		K &k = ks[i];
		string name = "KEY_ALT_" + k.name;
		fout << "#define    " << name << "    " << "0x" << k.smm[o] << k.asc[o] << endl;
	}
	fout << endl << endl << "#endif" << endl;
	return 0;
}
