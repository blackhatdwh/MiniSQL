#ifndef __CATALOGMANAGER_H__
#define __CATALOGMANAGER_H__

#include <string>
#include <vector>
#include <iostream>
#include <fstream>	
#define EMPTY '@'
#define END '#'

enum { INT, CHAR, FLOAT };

struct Attribute
{
	std::string name;
	int type; //INT FLOAT CHAR
	int length;
	bool isPrimaryKey;
	bool isUnique;
	Attribute(){}
	Attribute(std::string n, int t, int l, bool isP, bool isU) : name(n), type(t),
		length(l), isPrimaryKey(isP), isUnique(isU)
	{}
};

//每张表都对应于一个文件
struct Table
{
	std::string tablename;
	int blockNum = 0;
	int attriNum = 0;
	int tupleLength = 0;
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
	void DisplayIndexCatalog();
	int GetColumnIndex(Table& table, Attribute &a);
	int GetColumnAmount(Table& tableinfo);

	friend class IndexManeger;
	friend class BPlusTree;
	friend class RecordManager;
	friend class BufferManager;
};
#endif
