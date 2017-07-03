#include "BufferManager.h"
#include <cstring>
#define NOTFOUND -1
#define MAX_LRU_VAL 100
#define EMPTY '@'
#define END '#'

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
	filename = "";
	blockoffset = 0;
	LRU_Val = 0;
	modified = false;
	locked = false;
	inuse = false;
	isFull = false;
	end = 0;
}

char *BufferBlock::contents(int position) {
	if (LRU_Val < MAX_LRU_VAL)
		LRU_Val = LRU_Val + 1; //��������֮��Ҫ���·��ʴ���
	return (char *)(block + position);
}
char *BufferBlock::contents() {
	if (LRU_Val < MAX_LRU_VAL)
		LRU_Val = LRU_Val + 1;
	return (char *)block;
}

BufferManager::BufferManager() {}
BufferManager::~BufferManager() {
	for (int i = 0; i < MAXBLOCKNUMBER; i++) {
		if (bufferblocks[i].modified)
			StoreToDiskBlock(i);
	}
}

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
	cout << bufferblocks[0].filename << " " << bufferblocks[0].blockoffset << endl;
	if ((BufferID = GetBufferID(filename, blockoffset)) != NOTFOUND)
		return BufferID;
	BufferID = GetEmptyBufferBlock();
	LoadToBufferBlock(filename, blockoffset, BufferID);
	return BufferID;
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
	bufferblocks[bufferID].inuse = true;

	// ����
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
		if (bufferblocks[i].LRU_Val <= min && !bufferblocks[i].locked) { //�ȿ�LRU_Val���ٿ�index��LRU_ԽСԽ�ã�
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

// ��ش���
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

// �ڱ��в���һ�����ݣ���Ҫ�õ���������еĵ�ַ
BufferPosition BufferManager::GetInsertPosition(Table& table)
{
	Position p;
	BufferPosition bp;

	// �������һ��blockҲû�У�����ĵ�ַ��0
	if (table.blockNum == 0) {
		bp.BufferID = GetEmptyBufferBlock();
		bp.offset = 0;
		return bp;
	}

	// �ҵ����table�洢��λ�ã��ļ��������������Ǳ������+.data�������ļ���׺��

	int tuplenum = BLOCKSIZE / table.tupleLength;
	// ƫ��������block������-1��������һ�����һ��block�������������ֻ��һ��block�����Ե�һ��block����û��װ������ô�ڵ�һ��block����Ѱ�Ҳ����λ��
	int blockoffset = table.blockNum - 1;
	// ���ص�Buffer
	int bufferID = Load(table.tablename + ".data", blockoffset);

	// �������block
	for (int i = 0; i < tuplenum; i++) {
		// ��i�����ݵ�λ��
		int pos = i*table.tupleLength;

		// �������λ���ǲ��ǿյ�(����ǿյ���ô����ط��ĵ�һ��λ�û�����ϱ�ʾ�յ��ַ�)���ǿ��ŵĻ��Ͱ���������λ�÷��ء�
		if ((bufferblocks[bufferID].contents())[pos] == END) {
			bp.BufferID = bufferID;
			bp.offset = pos;
			return bp;
		}
	}

	// ������һ��block�Ѿ����˵Ļ���������һ��
	bp.BufferID = GetEmptyBufferBlock();
	bp.offset = 0;

	return bp;
}

void BufferManager::Access(int BufferID)
{
	bufferblocks[BufferID].LRU_Val++;
}

//��ָ���ļ���block��Buffer��ɾ��
void BufferManager::UnUseBufferBlocks(std::string filename)
{
	for (int i = 0; i < MAXBLOCKNUMBER; i++) {
		if (bufferblocks[i].filename == filename) {
			if (bufferblocks[i].modified)
				StoreToDiskBlock(i);//���޸ľʹ��ȥ
			bufferblocks[i].clear();
		}
	}
}