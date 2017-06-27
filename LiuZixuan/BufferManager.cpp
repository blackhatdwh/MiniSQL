#include "BufferManager.h"
#include <cstring>

BufferBlock::BufferBlock()
{
	clear();
}

BufferBlock::~BufferBlock()
{
}

void BufferBlock::clear()
{
	memset(block, 0, BLOCKSIZE+1);
	LRU_Val = 0;
	used = 0;
}

char *BufferBlock::contents(int position) const {
	return (char *)(block + position);
}
char *BufferBlock::contents() const {
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

// load the file from disk to buffer
void BufferManager::LoadToBufferBlock(std::string filename, int blockoffset, int bufferID)
{
	std::ifstream in(filename);
	if (!in) {
		std::cerr << "Failed to load the block, no such file named " << filename << std::endl;
	}
	bufferblocks[bufferID].clear();
	bufferblocks[bufferID].used = 1;
	bufferblocks[bufferID].blockoffset = blockoffset;
	bufferblocks[bufferID].filename = filename;

	// 读入
	in.seekg(BLOCKSIZE*blockoffset, in.beg);
	in.read((char *)bufferblocks[bufferID].contents(), BLOCKSIZE);

	in.close();
}


int BufferManager::GetEmptyBufferBlock()
{
	int bid = 0;
	int min = bufferblocks[0].LRU_Val;
	for (int i = 0; i < MAXBLOCKNUMBER; ++i) {
		if (!bufferblocks[i].used)
			return i;
		if (bufferblocks[i].LRU_Val <= min) { //先看LRU_Val，再看index（LRU_越小越好）
			min = bufferblocks[i].LRU_Val;
			bid = i;
		}
	}
	bufferblocks[bid].clear();
	StoreToDiskBlock(bid);
	return bid;
}

// 存回磁盘
void BufferManager::StoreToDiskBlock(int bufferID)
{
	std::string filename = bufferblocks[bufferID].filename;
	std::ofstream fout(filename);
	if (!fout) {
		std::cerr << "Failed to write back to the file " << filename << std::endl;
	}

	fout.seekp(BLOCKSIZE*bufferblocks[bufferID].blockoffset, fout.beg);
	fout.write(bufferblocks[bufferID].contents(), BLOCKSIZE);

	fout.close();
}

int BufferManager::GetEmptyBufferBlock(std::string except)
{
	int bid = NOTFOUND;
	int min = bufferblocks[0].LRU_Val;
	for (int i = 0; i < MAXBLOCKNUMBER; ++i) {
		if (bufferblocks[i].filename != except) {
			if (!bufferblocks[i].used)
				return i;
			if (bufferblocks[i].LRU_Val <= min) {
				min = bufferblocks[i].LRU_Val;
				bid = i;
			}
		}
	}

	if (bid != NOTFOUND) {
		bufferblocks[bid].clear();
		StoreToDiskBlock(bid);
	}

	return bid;
}

// 拿到插入的地址
Position BufferManager::GetInsertPosition(Table& table)
{
	Position ip;

	// 如果表中一个block也没有，那么就申请一个block，这个block的地址就是插入的地址
	if (!table.blockNum) {
		// 加上一个block，插入记录的位置就是这个block的开头
		ip.bufferID = ApplyBufferBlock(table);
		ip.position = 0;

		// 就是它了
		return ip;
	}

	// 找到这个table存储的位置，文件名的命名规则是表的名字+.data（这是文件后缀）
	std::string filename = table.tablename + ".data";

	// 一条数据占据的空间大小
	int singleLength = table.tupleLength + 1;

	// 一个block能够写入的数据的条数，这里是一种简单的做法，剩余的空间不管
	int recordamount = BLOCKSIZE / singleLength;

	// 偏移量就是block的数量-1（先搜索一下最后一个block），例如这个表只有一个block，所以第一个block可能没有装满，那么在第一个block里面寻找插入的位置
	int blockoffset = table.blockNum - 1;

	// 看看这个文件有没有被读进buffer中
	int bufferID = GetBufferID(filename, blockoffset);

	// 如果没有的话，就找一块空的block，将其load进来
	if (bufferID == -1) {
		bufferID = GetEmptyBufferBlock();
		LoadToBufferBlock(filename, blockoffset, bufferID);
	}

	// 处理这个block
	for (int i = 0; i < recordamount; i++) {
		// 第i条数据的位置
		int pos = i*singleLength;

		// 看看这个位置是不是空的(如果是空的娜美这个地方的第一个位置机会放上表示空的字符)，是空着的话就把这个插入的位置返回。
		if ((bufferblocks[bufferID].contents())[pos] == EMPTY) {
			ip.bufferID = bufferID;
			ip.position = pos;
			return ip;
		}
	}

	// 如果最后一个block已经满了的话就再申请一个
	ip.bufferID = ApplyBufferBlock(table);
	ip.position = 0;

	return ip;
}



// 重载函数，根据索引申请一个block
int BufferManager::ApplyBufferBlock(Index& index) {
	// 索引文件的命名方式
	std::string filename = index.indexname + ".index";
	// 申请一个block以加载索引
	int bufferID = GetEmptyBufferBlock(filename);
	bufferblocks[bufferID].used = 1;
	bufferblocks[bufferID].filename = filename;
	bufferblocks[bufferID].blockoffset = index.blockNum++;
	return bufferID;
}

int BufferManager::ApplyBufferBlock(Table& table) {
	int bufferID;
	std::string filename = table.tablename + ".data";
	std::fstream fin(filename.c_str(), std::ios::in);
	for (int i = 0; i<table.blockNum; i++)
		if (GetBufferID(filename, i) == -1) {
			bufferID = GetEmptyBufferBlock(filename);
			LoadToBufferBlock(filename, i, bufferID);
		}
	fin.close();
	return bufferID;
}

void BufferManager::UnUseBufferBlock(std::string filename) {
	for (int i = 0; i < MAXBLOCKNUMBER; i++)
		if (bufferblocks[i].filename == filename)
			bufferblocks[i].used = 0;
}

void BufferManager::UseBufferBlock(int bufferID)
{
	bufferblocks[bufferID].used = 1;
	bufferblocks[bufferID].LRU_Val++;
}
