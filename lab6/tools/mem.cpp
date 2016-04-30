#include <iostream>
using namespace std;

struct MemBlock{
	bool used;
	int left, right; //[left,right)
	int next;
	MemBlock():used(false){}
};

const int MaxBlockNum = 255;
const int MemoryStart = 10;
const int MemoryEnd = 100;
class Memory{
public:
	Memory(){
		data[0].used = true;
		data[0].next = 1;
		data[0].left = -1;
		data[0].right = -1;

		data[1].used = true;
		data[1].left = MemoryStart;
		data[1].right = MemoryEnd;
		data[1].next = MaxBlockNum;

		data[MaxBlockNum].used = true;
		data[MaxBlockNum].left = MemoryEnd + 1;
		data[MaxBlockNum].right = MemoryEnd + 1;
	};
	int allocate(int needSize){
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
	void free(int addr, int freeSize){
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
	void Print(){
		for (int p = data[0].next;p != MaxBlockNum;p = data[p].next){
			cout << "[" << data[p].left << ", " << data[p].right << ")" << ", ";
		}
		cout << endl;
	}
private:
	MemBlock data[MaxBlockNum + 1];
};

int main(){
	Memory mem;
	mem.Print();
	cout << "+: allocate size ; -: free addr size" << endl;
	while(1){
		char c;
		int size;
		int addr;
		cin >> c;
		if (c == '+'){
			cin >> size;
			cout << "addr: " << mem.allocate(size) << endl;
		}else{
			if (c == '-'){
				cin >> addr >> size; 
				mem.free(addr,size);
			}
		}
		mem.Print();
	}
	return 0;
}
