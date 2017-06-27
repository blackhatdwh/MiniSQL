#ifndef __BUFFERMANAGER_H__
#define __BUFFERMANAGER_H__

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "CatalogManager.h" // 表的信息定义在Catalog模块

#define MAXBLOCKNUMBER 1000
#define BLOCKSIZE 4096
#define NOTFOUND -1
#define EMPTY 0

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
	
public:
	friend class RecordManager;
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
	//friend class IndexManager;
	//Add some B+ tree class friends here

	
	BufferManager();
	virtual ~BufferManager();
};

#endif
