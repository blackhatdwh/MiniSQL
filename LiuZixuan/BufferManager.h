#ifndef __BUFFERMANAGER_H__
#define __BUFFERMANAGER_H__

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "CatalogManager.h" // �����Ϣ������Catalogģ��
#include "b_plus_tree.h"
#include "BufferManager.h"
#include "IndexManager.h"

#define MAXBLOCKNUMBER 1000
#define BLOCKSIZE 4096
#define NOTFOUND -1
#define EMPTY '#'

class BufferBlock
{
public:
	BufferBlock();
	~BufferBlock();
	char *contents();
	char *contents(int position);
	std::string filename;
	int blockoffset;
	bool modified;
	bool locked;
	bool inuse;
	int LRU_Val;
	void clear();
	int end;
	bool isFull;
private:
	char block[BLOCKSIZE+1];
};

typedef Position int;


struct BufferPosition
{
	int BufferID;
	int offset;
};

class BufferManager {
private:
	BufferBlock bufferblocks[MAXBLOCKNUMBER];//buffer memory
	
public:
	friend class RecordManager;
	int GetBufferID(std::string filename, int blockoffset) const; //����ĳ�ļ�block��buffer block���ĸ�λ��
	void LoadToBufferBlock(std::string filename, int blockoffset, int bufferID);//���ļ����ص�ĳ��buffer block��ȥ
	void StoreToDiskBlock(int bufferID); //��ָ����blockд�ص�������ȥ
	//void UseBufferBlock(int bufferID);
	void UnUseBufferBlocks(std::string filename); //��ָ����block����Ϊû��ʹ��
	int Load(std::string filename, int blockoffset);
	BufferPosition GetInsertPosition(Table& table);
	//Add some B+ tree class friends here

	BufferManager();
	virtual ~BufferManager();

	friend class IndexManeger;
	friend class BPlusTree;
	friend class CatalogManager;
	friend class RecordManager;

private:
	int GetEmptyBufferBlock();//ApplyBufferBlock���ӳ���
	int GetEmptyBufferBlock(std::string filename);
	
};

#endif
