#ifndef _OSMEMORY_H_
#define _OSMEMORY_H_

#include <stdint.h>
#include "io.h"

//16位下GCC不支持类Class?
//
//这个内存管理还有一点小问题， 会产生碎片

const uint16_t BOX_SIZE = 512;
const uint16_t BOX_NUM = 512;
uint16_t SPACE_PROG_SEGMENT;
#define GET_PROG_SEGMENT asm volatile("int 0x21;":"=a"(SPACE_PROG_SEGMENT):"a"(0x0400))



struct MemBlock{
	bool used;
	uint16_t left, right; //[left,right)
	uint16_t next;
};

const uint16_t MaxBlockNum = 255;
const uint16_t SPACE_SIZE = 0x4000;
MemBlock memdata[MaxBlockNum + 1];
uint16_t MemoryEnd;

void mem_init(){
	for (uint16_t i = 0;i < MaxBlockNum + 1;++i)memdata[i].used = false;
	GET_PROG_SEGMENT;

	memdata[0].used = true;
	memdata[0].next = 1;
	memdata[0].left = SPACE_PROG_SEGMENT - 1;
	memdata[0].right = SPACE_PROG_SEGMENT - 1;

	memdata[1].used = true;
	MemoryEnd = SPACE_PROG_SEGMENT + SPACE_SIZE;
	memdata[1].left = SPACE_PROG_SEGMENT;
	memdata[1].right = SPACE_PROG_SEGMENT + SPACE_SIZE;
	memdata[1].next = MaxBlockNum;

	memdata[MaxBlockNum].used = true;
	memdata[MaxBlockNum].left = MemoryEnd + 1;
	memdata[MaxBlockNum].right = MemoryEnd + 1;
};

__attribute__((regparm(1)))
uint16_t mem_allocate(uint16_t needSize){
	//使用最佳适应算法, 每次找最小的分区
	bool first = true;
	uint16_t blockSize = 0;
	uint16_t lastp = 0;
	uint16_t goodlastp = 0;
	uint16_t goodp = 0;
	for (uint16_t p = memdata[0].next;p != MaxBlockNum;lastp = p, p = memdata[p].next){
		uint16_t u = memdata[p].right - memdata[p].left;
		if (u < needSize)continue;
		if (first || u < blockSize){
			first = false;
			goodp = p;
			goodlastp = lastp;
			blockSize = u;
	 	}
	} 
	if (first)return 0xFFFF; // 无法分配
	uint16_t addr = memdata[goodp].left;
	//从链表中删除空间
	if (blockSize == needSize){
		//刚好删除
		memdata[goodp].used = false;
		memdata[goodlastp].next = memdata[goodp].next; 
	}else{
		//大于所要申请的空间
		//取前面部分
		memdata[goodp].left += needSize;
	}
	return addr;
}

__attribute__((regparm(2)))
void mem_free(uint16_t addr, uint16_t freeSize){
	//释放空间
	uint16_t lastp = 0;
	uint16_t p;
	for (p = memdata[lastp].next;p != MaxBlockNum;lastp = p, p = memdata[p].next){
		//找到>=上个节点的right值的点
		if (addr >= memdata[lastp].right && addr < memdata[p].left)break;
 	} 
	//这时, addr >= memdata[lastp].right
	if (addr == memdata[lastp].right){
		//与上一个节点融合
		//lastp 一定不为0
		if (addr + freeSize >= memdata[p].left){
			//与p节点融合
			if (p != MaxBlockNum){
				memdata[lastp].next = memdata[p].next;
				memdata[p].used = false;
				memdata[lastp].right = memdata[p].right;
			}else{
				memdata[lastp].right = MemoryEnd;
		 	}
		}else{
			memdata[lastp].right = addr + freeSize;
	 	}
	}else{
		//与上一个节点不融合
		//lastp 可能为0
		if (addr + freeSize >= memdata[p].left && p != MaxBlockNum){
			//与p节点融合
			memdata[p].left = addr;
		}else{
			//与前后都不融合
			uint16_t e;
			for (e = 1;e < MaxBlockNum;++e){
				if (!memdata[e].used)break;
			}
			memdata[lastp].next = e;
			memdata[e].used = true;
			memdata[e].next = p;
			memdata[e].left = addr;
			memdata[e].right = addr + freeSize;
		} 
	} 
} 



#endif
