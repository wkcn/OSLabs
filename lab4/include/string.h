#ifndef _STRING_H_
#define _STRING_H_

#include "defines.h"

__attribute__((regparm(1)))
osi strlen(const char *s){
	osi i = 0;
	while(*(s++))i++;
	return i;
}

__attribute__((regparm(2)))
int strcmp(const char *astr,const char *bstr){
	// = 0
	// < -1
	// > 1
	while ((*astr) && (*bstr)){
		if (*astr != *bstr){
			if (*astr < *bstr)return -1;
			return 1;
		}
		++astr;
		++bstr;
	}
	return (*astr) - (*bstr);
}

#endif
