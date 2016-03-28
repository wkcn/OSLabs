#ifndef _STRING_H_
#define _STRING_H_
#include "defines.h"

osi strlen(char *s){
	osi i = 0;
	while(*s)i++;
	return i;
}

osi strcmp(char *astr, char *bstr){
	// = 0
	// < -1
	// > 1
	while (*astr != 0 || *bstr != 0){
		if (*astr != *bstr){
			if (*astr < *bstr)return -1;
			return 1;
		}
	}
	return 0;
}

#endif
