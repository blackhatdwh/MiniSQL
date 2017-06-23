#ifndef __BUFFERMANAGER_H__
#define __BUFFERMANAGER_H__

// Buffer Manager

// Buffer Manager负责缓冲区的管理，主要功能有：
// 1.	根据需要，读取指定的数据到系统缓冲区或将缓冲区中的数据写出到文件；
// 2.	实现缓冲区的替换算法，当缓冲区满时选择合适的页进行替换；
// 3.	记录缓冲区中各页的状态，如是否被修改过等；
// 4.	提供缓冲区页的pin功能，及锁定缓冲区的页，不允许替换出去。
// 为提高磁盘I/O操作的效率，缓冲区与文件系统交互的单位是块，块的大小应为文件系统与磁盘交互单位的整数倍，一般可定为4KB或8KB。

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "CatalogManager.h" // 表的信息定义在Catalog模块
#include "Macro.h"

#define MAXBLOCKNUMBER 1000	//should be 10000
#define BLOCKSIZE 4096	//should be 4096
#define NOTFOUND -1


class BufferBlock
{
public:
	BufferBlock();
	~BufferBlock();
	char *contents() const;
	char *contents(int position) const;
	std::string filename;
	int blockoffset;
	bool used;
	int LRU_Val;
	void clear();
private:
	char block[BLOCKSIZE+1];
};

struct Position
{
	int bufferID;
	int position;
};


class BufferManager {
private:
	BufferBlock bufferblocks[MAXBLOCKNUMBER];//buffer memory
	int GetBufferID(std::string filename, int blockoffset) const; //查找某文件block在buffer block的哪个位置
	void LoadToBufferBlock(std::string filename, int blockoffset, int bufferID);//将文件加载到某个buffer block中去
	void StoreToDiskBlock(int bufferID);//写回到磁盘中
	void UseBufferBlock(int bufferID);
	void UnUseBufferBlock(std::string filename); //将指定的block设置为没被使用
	int GetEmptyBufferBlock();//ApplyBufferBlock的子程序
	int GetEmptyBufferBlock(std::string filename);
	int ApplyBufferBlock(Index& index);//申请buffer块用于加载文件
	int ApplyBufferBlock(Table& table);
	Position GetInsertPosition(Table& table);
public:
	friend class RecordManager;
	friend class IndexManager;
	//Add some B+ tree class friends here
	
	BufferManager();
	virtual ~BufferManager();
};

#endif