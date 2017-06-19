#ifndef __CATALOGMANAGER_H__
#define __CATALOGMANAGER_H__

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include "Macro.h"

// Catalog Manager负责管理数据库的所有模式信息，包括：
// 1.	数据库中所有表的定义信息，包括表的名称、表中字段（列）数、主键、定义在该表上的索引。
// 2.	表中每个字段的定义信息，包括字段类型、是否唯一等。
// 3.	数据库中所有索引的定义，包括所属表、索引建立在那个字段上等。
//另外
// 4.	数据表中的记录条数及空记录串的头记录号。
// 5.	数据库内已建的表的数目。
// Catalog Manager还必需提供访问及操作上述信息的接口，供Interpreter和API模块使用。		

struct Attribute
{
	std::string name;
	int type; //INT FLOAT CHAR
	int length;
	bool isPrimaryKey;
	bool isUnique;
	Attribute();
	Attribute(std::string n, int t, int l, bool isP, bool isU);
};

//每张表都对应于一个文件
struct Table
{
	std::string tablename;   //all the datas is store in file name.table
	int blockNum;	//number of block the datas of the table occupied in the file name.data
	int attriNum;	//the number of attributes in the tables
	int tupleLength;	//total length of one record, should be equal to sum(attributes[i].length)
	int tupleNum;
	std::vector<Attribute> attributes;
};

struct Index
{
	std::string indexname;	//文件名
	std::string tablename;	//索引对应的表
	int column = 0;			//根据哪一个属性建立索引排序(默认主键是第一列)
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
	void createTable(Table& table);
	void createIndex(Index index);
	void dropTable(Table table);
	void dropIndex(Index index);
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
	void ShowTableCatalog();
	void ShowIndexCatalog();
	int CatalogManager::GetColumnIndex(Table& table, Attribute &a)
	int GetColumnAmount(Table& tableinfo);
};

#endif
