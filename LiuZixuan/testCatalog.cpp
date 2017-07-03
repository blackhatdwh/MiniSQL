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

class CatalogManager {
private:
	// 存储表的容器
	std::vector<Table> tables;
	// 数据库中的表的数量，应该与table.size()一致。
	int tableNum;
	// 存储索引的容器
	std::vector<Index> indices;
	// 索引的数量
	int indexNum;
private:
	void LoadTableCatalog();
	void LoadIndexCatalog();

	// 把这些东西存回去
	void StoreTableCatalog();
	void StoreIndexCatalog();
public:
	CatalogManager();
	~CatalogManager();
	
	int CreateTable(Table &table);
	int CreateIndex(Index &index);
	void DropTable(std::string tablename);
	void DropIndex(std::string index_name);
	bool ExistTable(std::string tablename);
	bool ExistIndex(std::string tablename, int column);
	bool ExistIndex(std::string indexname);
	Table getTableInfo(std::string tablename);
	Index getIndexInfo(std::string tablename, int column);
	Index getIndexInfo(std::string indexName);
	void SplitDataItem(Table &t, Tuple &tuple, std::string item);

	void DisplayTableCatalog();
	int GetColumnIndex(Table& table, Attribute &a);
	int GetColumnAmount(Table& tableinfo);

	friend class IndexManeger;
	friend class BPlusTree;
	friend class RecordManager;
	friend class BufferManager;
};

void CatalogManager::LoadTableCatalog() {
	// 文件名
	std::string filename = "table.catalog";
	std::ifstream fin(filename);
	
	if (!fin) {
		cout << "Can't open the file named" << filename << endl;
	}

	// 文件的第一个数据是tableNum
	fin >> tableNum;

	for (int i = 0; i < tableNum; i++) {
		
		Table temp_table;
		fin >> temp_table.tablename;
		fin >> temp_table.blockNum;
		fin >> temp_table.attriNum;
		fin >> temp_table.tupleLength;

		for (int j = 0; j < temp_table.attriNum; j++) {
			Attribute temp_attri;
			// 属性的内容有名字、类型、长度、是否是主键、是否是唯一的。
			fin >> temp_attri.name;
			fin >> temp_attri.type;
			fin >> temp_attri.length;
			fin >> temp_attri.isPrimaryKey;
			fin >> temp_attri.isUnique;
			temp_table.attributes.push_back(temp_attri);
		}
		tables.push_back(temp_table);
	}
	fin.close();
}

void CatalogManager::LoadIndexCatalog() {
	// 文件名
	const std::string filename = "index.catalog";
	std::fstream fin(filename);
	// 第一个数据是index的数量
	fin >> indexNum;
	for (int i = 0; i < indexNum; i++)
	{//fill in the vector of indices
		Index temp_index;
		// index的内容有它的名字、表的名字、列、列的长度、block的数量
		fin >> temp_index.indexname;
		fin >> temp_index.blockNum;
		fin >> temp_index.tablename;
		fin >> temp_index.column; //建立索引的那一列
		fin >> temp_index.columnLength;//那一列一个元素的长度
		indices.push_back(temp_index);
	}
	fin.close();
}

void CatalogManager::StoreTableCatalog() {
	std::string filename = "table.catalog";
	std::ofstream fout(filename);

	fout << tableNum << std::endl;
	for (int i = 0; i < tableNum; i++)
	{
		fout << tables[i].tablename << " ";
		fout << tables[i].blockNum << " ";
		fout << tables[i].attriNum << " ";
		fout << tables[i].tupleLength << std::endl;

		for (int j = 0; j < tables[i].attriNum; j++) {
			fout << tables[i].attributes[j].name << " ";
			fout << tables[i].attributes[j].type << " ";
			fout << tables[i].attributes[j].length << " ";
			fout << tables[i].attributes[j].isPrimaryKey << " ";
			fout << tables[i].attributes[j].isUnique << endl;
		}
	}
	fout.close();
}

// 把这些东西存回去
void CatalogManager::StoreIndexCatalog() {
	std::string filename = "index.catalog";
	ofstream fout(filename);
	fout << indexNum << std::endl;
	for (int i = 0; i < indexNum; i++)
	{
		fout << indices[i].indexname << " ";
		fout << indices[i].blockNum << " ";
		fout << indices[i].tablename << " ";
		fout << indices[i].column << " ";
		fout << indices[i].columnLength << " " << endl;
	}
	fout.close();
}

CatalogManager::CatalogManager() {
	LoadTableCatalog();
	LoadIndexCatalog();
}

CatalogManager::~CatalogManager() {
	StoreTableCatalog();
	StoreIndexCatalog();
}

int CatalogManager::CreateTable(Table &table) {
	if (!ExistTable(table.tablename)) {
		tableNum++;
		tables.push_back(table);
		return 1;
	}
	return 0;
}

int CatalogManager::CreateIndex(Index &index) {
	if (!ExistIndex(index.indexname)) {
		indexNum++;
		indices.push_back(index);
		return 1;
	}
	return 0;
}

void CatalogManager::DropTable(std::string tablename) {

	//遍历找到之后删除
	for (int i = tableNum - 1; i >= 0; i--) {
		if (tables[i].tablename == tablename) {
			tables.erase(tables.begin() + i);
			tableNum--;
			break;//默认相同名字的表只有一张
		}
	}

	for (int i = indexNum - 1; i >= 0; i--) {
		if (indices[i].tablename == tablename) {
			indices.erase(indices.begin() + i);
			indexNum--;
		}
	}
}

void CatalogManager::DropIndex(std::string index_name) {
	for (int i = indexNum - 1; i >= 0; i--) {
		if (indices[i].indexname == index_name) {
			indices.erase(indices.begin() + i);
			indexNum--;
			break;
		}
	}
}

bool CatalogManager::ExistTable(std::string tablename) {
	int i;
	for (i = 0; i<tables.size(); i++) {
		if (tables[i].tablename == tablename)
			return true;
	}
	return false;
}

bool CatalogManager::ExistIndex(std::string tablename, int column) {
	int i;
	for (i = 0; i < indices.size(); i++) {
		if (indices[i].tablename == tablename && indices[i].column == column)
			return true;
	}
	return false;
}

bool CatalogManager::ExistIndex(std::string indexname) {
	int i;
	for (i = 0; i <indices.size(); i++) {
		if (indices[i].indexname == indexname)
			return true;
	}
	return false;
}

Table CatalogManager::getTableInfo(std::string tablename) {
	int i;
	Table t;//默认的空表
	for (i = 0; i<tableNum; i++) {
		if ((tables[i].tablename) == tablename) {
			return tables[i];
		}
	}
	return t;
}

Index CatalogManager::getIndexInfo(std::string indexName) {
	int i;
	Index idx;
	for (i = 0; i < tableNum; i++) {
		if (indices[i].indexname == indexName)
			return indices[i];
	}
	return idx;
	
}

Index CatalogManager::getIndexInfo(std::string tablename, int column)
{

}

// 根据属性返回属性所在的列的序号
int CatalogManager::GetColumnIndex(Table& table, Attribute &a)
{
	for (auto it = table.attributes.begin(); it != table.attributes.end(); ++it) {
		if (a.name == it->name)
			return (it- table.attributes.begin());
	}
	return NOTFOUND;
}

// 返回列的数量
int CatalogManager::GetColumnAmount(Table& tableinfo) {
	return tableinfo.attributes.size();
}

//将item分解，形成一个tuple
void CatalogManager::SplitDataItem(Table &t, Tuple &tuple, std::string item)
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

void CatalogManager::DisplayTableCatalog()
{
	cout << tableNum << "\n";
	for (int i = 0; i < tableNum; i++)
	{
		cout << tables[i].tablename << " ";
		cout << tables[i].blockNum << " ";
		cout << tables[i].attriNum << " ";
		cout << tables[i].tupleLength << std::endl;

		for (int j = 0; j < tables[i].attriNum; j++) {
			cout << tables[i].attributes[j].name << " ";
			cout << tables[i].attributes[j].type << " ";
			cout << tables[i].attributes[j].length << " ";
			cout << tables[i].attributes[j].isPrimaryKey << " ";
			cout << tables[i].attributes[j].isUnique << endl;
		}
	}
}

BufferManager bum;
CatalogManager cam;

int main()
{
	Attribute ID("student_id", INT, sizeof(int), 1, 1);
    Attribute Name("student_name", CHAR, 50, 0, 0);
    // Attribute Fen("GPA", FLOAT, sizeof(float), 0, 0);
    Table t;
    t.tablename = "student2";
    t.blockNum = 1;
    t.attriNum = 2;
    t.tupleLength = sizeof(int)+50;
    t.tupleNum = 10;
    t.attributes.push_back(ID);
    t.attributes.push_back(Name);
    // t.attributes.push_back(Fen);
    cout << cam.CreateTable(t) << endl;
    return 0;
}
