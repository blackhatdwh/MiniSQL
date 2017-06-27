#ifndef __CATALOGMANAGER_H__
#define __CATALOGMANAGER_H__

#include <string>
#include <vector>
#include <iostream>
#include <fstream>	

enum {INT, FLOAT, CHAR};

struct Attribute
{
	std::string name;
	int type; //INT FLOAT CHAR
	int length;
	bool isPrimaryKey;
	bool isUnique;
	Attribute();
	Attribute(std::string name, int type, int length, bool isPrimary, bool isUnique);
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

	Table();
};

struct Index
{
	std::string indexname;
	std::string tablename;
	int column = 0;
	int columnLength = 0;
	int blockNum = 0;
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
	
	void createTable(Table &table);
	void createIndex(Index &index);
	void dropTable(std::string tablename);
	void dropIndex(std::string index_name);
	void update(Table& tableinfor);
	void update(Index& index);
	bool ExistTable(std::string tablename);
	bool ExistIndex(std::string tablename, int column);
	bool ExistIndex(std::string indexname);
	Table getTableInfo(std::string tablename);
	Index getIndexInfo(std::string tablename, int column);
	Index getIndexInfo(std::string indexName);
	void splitDataItem(Table &t, Tuple &tuple, std::string item);

	void ShowCatalog();
	void ShowTableCatalog() {}
	void ShowIndexCatalog() {}
	int GetColumnIndex(Table& table, Attribute &a);
	int GetColumnAmount(Table& tableinfo);
};

#endif
