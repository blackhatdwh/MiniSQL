#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <vector>
#include <cstring>
#define NOTFOUND -1
#define MAX_LRU_VAL 100
#define EMPTY '@'
#define END '#'

using namespace std;


enum {INT, CHAR, FLOAT};

struct Attribute
{
	std::string name;
	int type; //INT FLOAT CHAR
	int length;
	bool isPrimaryKey;
	bool isUnique;
	Attribute() {}
	Attribute(std::string n, int t, int l, bool isP, bool isU) : name(n), type(t),
    length(l), isPrimaryKey(isP), isUnique(isU)
    {}
};

//每张表都对应于一个文件
struct Table
{
	std::string tablename;   //all the datas is store in file name.table
	int blockNum = 0;	//number of block the datas of the table occupied in the file name.data
	int attriNum = 0;	//the number of attributes in the tables
	int tupleLength = 0;	//total length of one record, should be equal to sum(attributes[i].length)
	int tupleNum = 0;
	std::vector<Attribute> attributes;
};

struct Index
{
	std::string indexname;
	int blockNum = 0;
	std::string tablename;
	int column = 0;
	int columnLength = 0;
};

struct Tuple {
	std::vector<std::string> columns;
};


inline string ReadData(char *s, int start, int length)
{
	string str;
    str.resize(length);
    for (int i = start; i < start+length; i++) {
        str[i-start] = s[i];
    }
	return str;
}


//将item分解，形成一个tuple
void SplitDataItem(Table &t, Tuple &tuple, std::string item)
{
	int start = 0;
	char s[500];
	for (auto it = t.attributes.begin(); it != t.attributes.end(); ++it) {
		switch (it->type) {
		case INT:
			_itoa(*(int *)(item.c_str() + start), s, 10);
			break;
		case CHAR:
			for (int j = start; j < start + it->length; j++) {
				s[j-start] = item[j];
			}
			break;
		case FLOAT:
			_itoa(*(float *)(item.c_str() + start), s, 10);
			break;
		default:
			cerr << "Wrong Type" << "\n";
		}

		start += it->length;

		tuple.columns.push_back(string(s));
	}
}

#define MAXBLOCKNUMBER 10
#define BLOCKSIZE 4096
#define NOTFOUND -1
#define EMPTY '@'
#define END '#'

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

typedef int Position;


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
	int GetBufferID(std::string filename, int blockoffset) const; //查找某文件block在buffer block的哪个位置
	void LoadToBufferBlock(std::string filename, int blockoffset, int bufferID);//将文件加载到某个buffer block中去
	void StoreToDiskBlock(int bufferID); //将指定的block写回到磁盘中去
	//void UseBufferBlock(int bufferID);
	void UnUseBufferBlocks(std::string filename); //将指定的block设置为没被使用
	int Load(std::string filename, int blockoffset);
	BufferPosition GetInsertPosition(Table& table);
    BufferBlock *GetBuffer() {
        return bufferblocks;
    }
    void Access(int BufferID);
	//Add some B+ tree class friends here

	BufferManager();
	virtual ~BufferManager();

	friend class IndexManeger;
	friend class BPlusTree;
	friend class CatalogManager;
//	friend class RecordManager;

private:
	int GetEmptyBufferBlock();//ApplyBufferBlock的子程序
	int GetEmptyBufferBlock(std::string filename);
	
};


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
		if ((bufferblocks[bufferID].contents())[pos] == END) {
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

void BufferManager::Access(int BufferID)
{
	bufferblocks[BufferID].LRU_Val++;
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

BufferManager bum;

int main()
{
    int bufferID = bum.Load("student.data", 0);
    cout << bufferID << "\n";
    bufferID = bum.Load("student.data", 0);
    cout << bufferID << "\n";
    
    
	int ins, offset;
	cout << "Your Instruction: ";
	while (cin >> ins) {
		if (ins == 1) {
			cout << "Please Input the blockoffset you want to load: ";
			cin >> offset;
			bufferID = bum.Load("student2.data", offset);
			cout << "The BufferID is " << bufferID << "\n";
		} else if (ins == 2){
			cout << "Please Input the Block ID you want to access: ";
			cin >> bufferID;
			bum.Access(bufferID);
			cout << bufferID << " is accessed" << "\n";
		} else if (ins == 3){
			bum.UnUseBufferBlocks("student2.data");
		} else {
			cout << "Please input the offset: ";
			cin >> offset;
			bufferID = bum.GetBufferID("student2.data", offset);
			cout << "The block's BufferID is " << bufferID << "\n";
		}
		cout << "Here is the Memory: " << "\n";
		for (int i = 0; i < MAXBLOCKNUMBER; i++) {
			cout << "Block " << i << ": ";
			cout << "LRU-" << bum.GetBuffer()[i].LRU_Val << "\n"; 
		}
		cout << "Your Instruction: ";
	}

    
    return 0;
}
