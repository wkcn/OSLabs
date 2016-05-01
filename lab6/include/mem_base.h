#ifndef _MEM_BASE_H_
#define _MEM_BASE_H_

#include <stdint.h>

struct MemBlock{
	bool used;
	uint16_t left, right; //[left,right)
	uint16_t next;
};

struct MemRecord{
	MemBlock *data;
	uint16_t MaxBlockNum;
};

__attribute__((regparm(3)))
void mem_init(MemRecord &mem, uint16_t left, uint16_t right){
	for (uint16_t i = 0;i < mem.MaxBlockNum + 1;++i)mem.data[i].used = false;

	mem.data[0].used = true;
	mem.data[0].next = 1;
	mem.data[0].left = left; 
	mem.data[0].right = left;

	mem.data[1].used = true;
	mem.data[1].left = left;
	mem.data[1].right = right;
	mem.data[1].next = mem.MaxBlockNum;

	mem.data[mem.MaxBlockNum].used = true;
	mem.data[mem.MaxBlockNum].left = right;
	mem.data[mem.MaxBlockNum].right = right;
};

__attribute__((regparm(2)))
uint16_t mem_allocate(MemRecord &mem, uint16_t needSize){
	//使用最佳适应算法, 每次找最小的分区
	bool first = true;
	uint16_t blockSize = 0;
	uint16_t lastp = 0;
	uint16_t goodlastp = 0;
	uint16_t goodp = 0;
	for (uint16_t p = mem.data[0].next;p != mem.MaxBlockNum;lastp = p, p = mem.data[p].next){
		uint16_t u = mem.data[p].right - mem.data[p].left;
		if (u < needSize)continue;
		if (first || u < blockSize){
			first = false;
			goodp = p;
			goodlastp = lastp;
			blockSize = u;
	 	}
	} 
	if (first)return 0xFFFF; // 无法分配
	uint16_t addr = mem.data[goodp].left;
	//从链表中删除空间
	if (blockSize == needSize){
		//刚好删除
		mem.data[goodp].used = false;
		mem.data[goodlastp].next = mem.data[goodp].next; 
	}else{
		//大于所要申请的空间
		//取前面部分
		mem.data[goodp].left += needSize;
	}
	return addr;
}

__attribute__((regparm(3)))
void mem_free(MemRecord &mem, uint16_t addr, uint16_t freeSize){
	//释放空间
	uint16_t lastp = 0;
	uint16_t p;
	for (p = mem.data[lastp].next;p != mem.MaxBlockNum;lastp = p, p = mem.data[p].next){
		//找到>=上个节点的right值的点
		if (addr >= mem.data[lastp].right && addr < mem.data[p].left)break;
 	} 
	//这时, addr >= memdata[lastp].right
	if (addr == mem.data[lastp].right && lastp != 0){
		//与上一个节点融合
		//lastp 一定不为0
		if (addr + freeSize >= mem.data[p].left){
			//与p节点融合
			if (p != mem.MaxBlockNum){
				mem.data[lastp].next = mem.data[p].next;
				mem.data[p].used = false;
				mem.data[lastp].right = mem.data[p].right;
			}else{
				mem.data[lastp].right = addr + freeSize;
		 	}
		}else{
			mem.data[lastp].right = addr + freeSize;
	 	}
	}else{
		//与上一个节点不融合
		//lastp 可能为0
		if (addr + freeSize >= mem.data[p].left && p != mem.MaxBlockNum){
			//与p节点融合
			mem.data[p].left = addr;
		}else{
			//与前后都不融合
			uint16_t e;
			for (e = 1;e < mem.MaxBlockNum;++e){
				if (!mem.data[e].used)break;
			}
			mem.data[lastp].next = e;
			mem.data[e].used = true;
			mem.data[e].next = p;
			mem.data[e].left = addr;
			mem.data[e].right = addr + freeSize;
		} 
	} 
} 


#endif
