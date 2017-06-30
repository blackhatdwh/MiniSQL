#include "BufferManager.h"
#include <cstring>
#define NOTFOUND -1
#define MAX_LRU_VAL 100
#define EMPTY '#'

extern BufferManager bfm;
extern CatalogManager cam;
extern RecordManager rem;
extern IndexManager idm;

BufferBlock::BufferBlock()
{
	clear();
}

BufferBlock::~BufferBlock()
{
}

void BufferBlock::clear()
{
	memset(block, 0, BLOCKSIZE);
	LRU_Val = 0;
	modified = false;
	locked = false;
	inuse = false;
	isFull = false;
	end = 0;
}

char *BufferBlock::contents(int position) {
	if (LRU_Val < MAX_LRU_VAL)
		LRU_Val = LRU_Val + 1; //访问内容之后要更新访问次数
	return (char *)(block + position);
}
char *BufferBlock::contents() {
	if (LRU_Val < MAX_LRU_VAL)
		LRU_Val = LRU_Val + 1;
	return (char *)block;
}

BufferManager::BufferManager() {}
BufferManager::~BufferManager() {}

// every block in the disk has a unique (filename, blockofset)
// so we can go through the buffer to check the blocks' filename and blockoffset
int BufferManager::GetBufferID(std::string filename, int blockoffset) const
{
	for (int i = 0; i < MAXBLOCKNUMBER; i++)
		if (bufferblocks[i].filename == filename && bufferblocks[i].blockoffset == blockoffset)
			return i;
	return NOTFOUND;
}


int BufferManager::Load(std::string filename, int blockoffset)
{
	int BufferID;
	if ((BufferID = GetBufferID(filename, blockoffset)) != NOTFOUND)
		return BufferID;
	return GetEmptyBufferBlock();
}

// load the file from disk to buffer
void BufferManager::LoadToBufferBlock(std::string filename, int blockoffset, int bufferID)
{
	std::ifstream in(filename);
	if (!in) {
		std::cerr << "Failed to load the block, no such file named " << filename << std::endl;
		return;
	}
	bufferblocks[bufferID].clear();
	bufferblocks[bufferID].blockoffset = blockoffset;
	bufferblocks[bufferID].filename = filename;

	// 读入
	in.seekg(BLOCKSIZE*blockoffset, in.beg);
	in.read((char *)bufferblocks[bufferID].contents(), BLOCKSIZE);

	in.close();
}


int BufferManager::GetEmptyBufferBlock()
{
	int BufferID = NOTFOUND;
	int min = MAX_LRU_VAL + 1;
	for (int i = 0; i < MAXBLOCKNUMBER; ++i) {
		if (!bufferblocks[i].inuse)
			return i;
		if (bufferblocks[i].LRU_Val <= min && !bufferblocks[i].locked) { //先看LRU_Val，再看index（LRU_越小越好）
			min = bufferblocks[i].LRU_Val;
			BufferID = i;
		}
	}

	if (BufferID == NOTFOUND)
		return NOTFOUND;

	if (bufferblocks[BufferID].modified)
		StoreToDiskBlock(BufferID);

	bufferblocks[BufferID].clear();
	return BufferID;
}

// 存回磁盘
void BufferManager::StoreToDiskBlock(int bufferID)
{
	std::string filename = bufferblocks[bufferID].filename;
	std::ofstream fout(filename);
	if (!fout) {
		std::cerr << "Failed to write back to the file " << filename << std::endl;
		return;
	}

	fout.seekp(BLOCKSIZE*bufferblocks[bufferID].blockoffset, fout.beg);
	fout.write(bufferblocks[bufferID].contents(), BLOCKSIZE);

	fout.close();
}

int BufferManager::GetEmptyBufferBlock(std::string except)
{
	int BufferID = NOTFOUND;
	int min = MAX_LRU_VAL + 1;
	for (int i = 0; i < MAXBLOCKNUMBER; ++i) {
		if (bufferblocks[i].filename != except) {
			if (!bufferblocks[i].inuse)
				return i;
			if (bufferblocks[i].LRU_Val <= min && !bufferblocks[i].locked) {
				min = bufferblocks[i].LRU_Val;
				BufferID = i;
			}
		}
	}

	if (BufferID == NOTFOUND)
		return NOTFOUND;

	if (bufferblocks[BufferID].modified)
		StoreToDiskBlock(BufferID);

	bufferblocks[BufferID].clear();
	return BufferID;
}

// 在表中插入一条数据，需要得到在这个表中的地址
BufferPosition BufferManager::GetInsertPosition(Table& table)
{
	Position p;
	BufferPosition bp;

	// 如果表中一个block也没有，插入的地址是0
	if (table.blockNum == 0) {
		bp.BufferID = GetEmptyBufferBlock();
		bp.offset = 0;
		return bp;
	}

	// 找到这个table存储的位置，文件名的命名规则是表的名字+.data（这是文件后缀）
	std::string filename = table.tablename + ".data";
	int tuplenum = BLOCKSIZE / table.tupleLength;
	// 偏移量就是block的数量-1（先搜索一下最后一个block），例如这个表只有一个block，所以第一个block可能没有装满，那么在第一个block里面寻找插入的位置
	int blockoffset = table.blockNum - 1;
	// 加载到Buffer
	int bufferID = Load(filename, blockoffset);

	// 处理这个block
	for (int i = 0; i < tuplenum; i++) {
		// 第i条数据的位置
		int pos = i*table.tupleLength;

		// 看看这个位置是不是空的(如果是空的那么这个地方的第一个位置机会放上表示空的字符)，是空着的话就把这个插入的位置返回。
		if ((bufferblocks[bufferID].contents())[pos] == EMPTY) {
			bp.BufferID = bufferID;
			bp.offset = pos;
			return bp;
		}
	}

	// 如果最后一个block已经满了的话就再申请一个
	bp.BufferID = GetEmptyBufferBlock();
	bp.offset = 0;

	return bp;
}

//将指定文件的block从Buffer中删除
void BufferManager::UnUseBufferBlocks(std::string filename)
{
	for (int i = 0; i < MAXBLOCKNUMBER; i++) {
		if (bufferblocks[i].filename == filename) {
			if (bufferblocks[i].modified)
				StoreToDiskBlock(i);//有修改就存回去
			bufferblocks[i].clear();
		}
	}
}

//// 重载函数，根据索引申请一个block
//int BufferManager::ApplyBufferBlock(Index& index) {
//	// 索引文件的命名方式
//	std::string filename = index.indexname + ".index";
//	// 申请一个block以加载索引
//	int bufferID = GetEmptyBufferBlock(filename);
//	bufferblocks[bufferID].used = 1;
//	bufferblocks[bufferID].filename = filename;
//	bufferblocks[bufferID].blockoffset = index.blockNum++;
//	return bufferID;
//}
//
//int BufferManager::ApplyBufferBlock(Table& table) {
//	int bufferID;
//	std::string filename = table.tablename + ".data";
//	std::fstream fin(filename.c_str(), std::ios::in);
//	for (int i = 0; i<table.blockNum; i++)
//		if (GetBufferID(filename, i) == -1) {
//			bufferID = GetEmptyBufferBlock(filename);
//			LoadToBufferBlock(filename, i, bufferID);
//		}
//	fin.close();
//	return bufferID;
//}