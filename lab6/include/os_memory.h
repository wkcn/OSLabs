#ifndef _OSMEMORY_H_
#define _OSMEMORY_H_

#include <stdint.h>
#include "port.h"

//16位下GCC不支持类Class?

const uint16_t BOX_SIZE = 512;
const uint16_t BOX_NUM = 512;
uint16_t SPACE_PROG_SEGMENT;
#define GET_PROG_SEGMENT asm volatile("int 0x21;":"=a"(SPACE_PROG_SEGMENT):"a"(0x0400))



struct MemBlock{
	bool used;
	int left, right; //[left,right)
	int next;
};

const int MaxBlockNum = 255;
const int SPACE_SIZE = 0x6000;
MemBlock data[MaxBlockNum + 1];
int MemoryEnd;

void mem_init(){
	for (int i = 0;i < SPACE_SIZE + 1;++i)data[i].used = false;
	data[0].used = true;
	data[0].next = 1;
	data[0].left = -1;
	data[0].right = -1;

	data[1].used = true;
	GET_PROG_SEGMENT;
	MemoryEnd = SPACE_PROG_SEGMENT + SPACE_SIZE;
	data[1].left = SPACE_PROG_SEGMENT;
	data[1].right = SPACE_PROG_SEGMENT + SPACE_SIZE;
	data[1].next = MaxBlockNum;

	data[MaxBlockNum].used = true;
	data[MaxBlockNum].left = MemoryEnd + 1;
	data[MaxBlockNum].right = MemoryEnd + 1;
};

__attribute__((regparm(1)))
	int mem_allocate(int needSize){
		//使用最佳适应算法, 每次找最小的分区
		bool first = true;
		int blockSize = 0;
		int lastp = 0;
		int goodlastp = 0;
		int goodp = 0;
		for (int p = data[0].next;p != MaxBlockNum;lastp = p, p = data[p].next){
			int u = data[p].right - data[p].left;
			if (u < needSize)continue;
			if (first || u < blockSize){
				first = false;
				goodp = p;
				goodlastp = lastp;
				blockSize = u;
			}
		}
		if (first)return -1; // 无法分配
		int addr = data[goodp].left;
		//从链表中删除空间
		if (blockSize == needSize){
			//刚好删除
			data[goodp].used = false;
			data[goodlastp].next = data[goodp].next; 
		}else{
			//大于所要申请的空间
			//取前面部分
			data[goodp].left += needSize;
		}
		return addr;
	}
__attribute__((regparm(2)))
	void mem_free(int addr, int freeSize){
		//释放空间
		int lastp = 0;
		int p;
		for (p = data[lastp].next;p != MaxBlockNum;lastp = p, p = data[p].next){
			//找到>=上个节点的right值的点
			if (addr >= data[lastp].right && addr < data[p].right)break;
		}
		//这时, addr >= data[lastp].right
		if (addr == data[lastp].right){
			//与上一个节点融合
			//lastp 一定不为0
			if (addr + freeSize >= data[p].left){
				//与p节点融合
				if (p != MaxBlockNum){
					data[lastp].next = data[p].next;
					data[p].used = false;
					data[lastp].right = data[p].right;
				}else{
					data[lastp].right = MemoryEnd;
				}
			}else{
				data[lastp].right = addr + freeSize;
			}
		}else{
			//与上一个节点不融合
			//lastp 可能为0
			if (addr + freeSize >= data[p].left){
				//与p节点融合
				data[p].left = addr;
			}else{
				//与前后都不融合
				int e;
				for (e = 1;e < MaxBlockNum;++e){
					if (!data[e].used)break;
				}
				data[lastp].next = e;
				data[e].used = true;
				data[e].next = p;
				data[e].left = addr;
				data[e].right = addr + freeSize;
			}
		}
	}



#endif
