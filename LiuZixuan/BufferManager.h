#ifndef __BUFFERMANAGER_H__
#define __BUFFERMANAGER_H__

// Buffer Manager

// Buffer Manager���𻺳����Ĺ�����Ҫ�����У�
// 1.	������Ҫ����ȡָ�������ݵ�ϵͳ�������򽫻������е�����д�����ļ���
// 2.	ʵ�ֻ��������滻�㷨������������ʱѡ����ʵ�ҳ�����滻��
// 3.	��¼�������и�ҳ��״̬�����Ƿ��޸Ĺ��ȣ�
// 4.	�ṩ������ҳ��pin���ܣ���������������ҳ���������滻��ȥ��
// Ϊ��ߴ���I/O������Ч�ʣ����������ļ�ϵͳ�����ĵ�λ�ǿ飬��Ĵ�СӦΪ�ļ�ϵͳ����̽�����λ����������һ��ɶ�Ϊ4KB��8KB��

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "CatalogManager.h" // �����Ϣ������Catalogģ��
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
public:
	friend class RecordManager;
	friend class IndexManager;
	//Add some B+ tree class friends here
	
	BufferManager();
	virtual ~BufferManager();
};

#endif