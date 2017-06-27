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

	// ����
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
		if (bufferblocks[i].LRU_Val <= min) { //�ȿ�LRU_Val���ٿ�index��LRU_ԽСԽ�ã�
			min = bufferblocks[i].LRU_Val;
			bid = i;
		}
	}
	bufferblocks[bid].clear();
	StoreToDiskBlock(bid);
	return bid;
}

// ��ش���
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

// �õ�����ĵ�ַ
Position BufferManager::GetInsertPosition(Table& table)
{
	Position ip;

	// �������һ��blockҲû�У���ô������һ��block�����block�ĵ�ַ���ǲ���ĵ�ַ
	if (!table.blockNum) {
		// ����һ��block�������¼��λ�þ������block�Ŀ�ͷ
		ip.bufferID = ApplyBufferBlock(table);
		ip.position = 0;

		// ��������
		return ip;
	}

	// �ҵ����table�洢��λ�ã��ļ��������������Ǳ������+.data�������ļ���׺��
	std::string filename = table.tablename + ".data";

	// һ������ռ�ݵĿռ��С
	int singleLength = table.tupleLength + 1;

	// һ��block�ܹ�д������ݵ�������������һ�ּ򵥵�������ʣ��Ŀռ䲻��
	int recordamount = BLOCKSIZE / singleLength;

	// ƫ��������block������-1��������һ�����һ��block�������������ֻ��һ��block�����Ե�һ��block����û��װ������ô�ڵ�һ��block����Ѱ�Ҳ����λ��
	int blockoffset = table.blockNum - 1;

	// ��������ļ���û�б�����buffer��
	int bufferID = GetBufferID(filename, blockoffset);

	// ���û�еĻ�������һ��յ�block������load����
	if (bufferID == -1) {
		bufferID = GetEmptyBufferBlock();
		LoadToBufferBlock(filename, blockoffset, bufferID);
	}

	// �������block
	for (int i = 0; i < recordamount; i++) {
		// ��i�����ݵ�λ��
		int pos = i*singleLength;

		// �������λ���ǲ��ǿյ�(����ǿյ���������ط��ĵ�һ��λ�û�����ϱ�ʾ�յ��ַ�)���ǿ��ŵĻ��Ͱ���������λ�÷��ء�
		if ((bufferblocks[bufferID].contents())[pos] == EMPTY) {
			ip.bufferID = bufferID;
			ip.position = pos;
			return ip;
		}
	}

	// ������һ��block�Ѿ����˵Ļ���������һ��
	ip.bufferID = ApplyBufferBlock(table);
	ip.position = 0;

	return ip;
}



// ���غ�����������������һ��block
int BufferManager::ApplyBufferBlock(Index& index) {
	// �����ļ���������ʽ
	std::string filename = index.indexname + ".index";
	// ����һ��block�Լ�������
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
