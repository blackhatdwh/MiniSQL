#ifndef __BUFFERMANAGER_H__
#define __BUFFERMANAGER_H__

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "CatalogManager.h" // �����Ϣ������Catalogģ��

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
	int GetBufferID(std::string filename, int blockoffset) const; //����ĳ�ļ�block��buffer block���ĸ�λ��
	void LoadToBufferBlock(std::string filename, int blockoffset, int bufferID);//���ļ����ص�ĳ��buffer block��ȥ
	void StoreToDiskBlock(int bufferID);//д�ص�������
	void UseBufferBlock(int bufferID);
	void UnUseBufferBlock(std::string filename); //��ָ����block����Ϊû��ʹ��
	int GetEmptyBufferBlock();//ApplyBufferBlock���ӳ���
	int GetEmptyBufferBlock(std::string filename);
	int ApplyBufferBlock(Index& index);//����buffer�����ڼ����ļ�
	int ApplyBufferBlock(Table& table);
	Position GetInsertPosition(Table& table);
	//friend class IndexManager;
	//Add some B+ tree class friends here

	
	BufferManager();
	virtual ~BufferManager();
};

#endif
