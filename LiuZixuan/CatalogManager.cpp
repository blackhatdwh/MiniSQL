#include "CatalogManager.h"
#include "BufferManager.h"
#include <string>
#include <vector>
#include <iostream>
#include <fstream>

using namespace std;



Attribute::Attribute()
{
	isPrimaryKey = false;
	isUnique = false;
}
Attribute::Attribute(string n, int t, int l, bool isP, bool isU)
	:name(n), type(t), length(l), isPrimaryKey(isP), isUnique(isU) {}

Table::Table() : blockNum(0), attriNum(0), tupleLength(0) {}

void CatalogManager::LoadTableCatalog() {
	// 文件名
	const std::string filename = "table.catlog";

	std::fstream fin(filename, std::ios::in);

	// 文件的第一个数据是tableNum
	fin >> tableNum;

	for (int i = 0; i < tableNum; i++)
	{
		//fill in the vector of tables
		// 一个table存储的数据是名字，属性的数量，block的数量
		Table temp_table;
		fin >> temp_table.tablename;
		fin >> temp_table.attriNum;
		fin >> temp_table.blockNum;

		for (int j = 0; j < temp_table.attriNum; j++)
		{
			//fill in the vector of temp_table.attributes
			Attribute temp_attri;
			// 属性的内容有名字、类型、长度、是否是主键、是否是唯一的。
			fin >> temp_attri.name;
			fin >> temp_attri.type;
			fin >> temp_attri.length;
			fin >> temp_attri.isPrimaryKey;
			fin >> temp_attri.isUnique;

			temp_table.attributes.push_back(temp_attri);
			temp_table.tupleLength += temp_attri.length;
		}

		tables.push_back(temp_table);
	}
	fin.close();
}

void CatalogManager::LoadIndexCatalog() {
	// 文件名
	const std::string filename = "index.catlog";
	std::fstream fin(filename, std::ios::in);
	// 第一个数据是index的数量
	fin >> indexNum;
	for (int i = 0; i < indexNum; i++)
	{//fill in the vector of indices
		Index temp_index;
		// index的内容有它的名字、表的名字、列、列的长度、block的数量
		fin >> temp_index.indexname;
		fin >> temp_index.tablename;
		fin >> temp_index.column;
		fin >> temp_index.columnLength;
		fin >> temp_index.blockNum;

		indices.push_back(temp_index);
	}
	fin.close();
}

void CatalogManager::StoreTableCatalog() {
	std::string filename = "table.catlog";
	std::fstream  fout(filename.c_str(), std::ios::out);

	// 将信息写回，这里可以看到两个层次一个是table的层次，第二个层次是属性的层次
	fout << tableNum << std::endl;
	for (int i = 0; i < tableNum; i++)
	{
		fout << tables[i].tablename << " ";
		fout << tables[i].attriNum << " ";
		fout << tables[i].blockNum << std::endl;

		for (int j = 0; j < tables[i].attriNum; j++)
		{
			fout << tables[i].attributes[j].name << " ";
			fout << tables[i].attributes[j].type << " ";
			fout << tables[i].attributes[j].length << " ";
			fout << tables[i].attributes[j].isUnique << " ";
			fout << tables[i].attributes[j].isPrimaryKey << endl;
		}
	}
	fout.close();
}

// 把这些东西存回去
void CatalogManager::StoreIndexCatalog() {
	std::string filename = "index.catlog";
	fstream fout(filename.c_str(), ios::out);
	fout << indexNum << std::endl;
	for (int i = 0; i < indexNum; i++)
	{
		fout << indices[i].indexname << " ";
		fout << indices[i].tablename << " ";
		fout << indices[i].column << " ";
		fout << indices[i].columnLength << " ";
		fout << indices[i].blockNum << endl;
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

void CatalogManager::createTable(Table &table) {
	tableNum++;
	for (int i = 0; i < table.attributes.size(); i++) {
		table.tupleLength += table.attributes[i].length;
	}
	tables.push_back(table);
}

void CatalogManager::createIndex(Index &index) {
	indexNum++;
	indices.push_back(index);
}

void CatalogManager::dropTable(std::string tablename) {
	for (int i = tableNum - 1; i >= 0; i--) {
		if (tables[i].tablename == tablename) {
			tables.erase(tables.begin() + i);
			tableNum--;
		}
	}
	for (int i = indexNum - 1; i >= 0; i--) {
		if (indices[i].tablename == tablename) {
			indices.erase(indices.begin() + i);
			indexNum--;
		}
	}
}

void CatalogManager::dropIndex(std::string index_name) {
	for (int i = indexNum - 1; i >= 0; i--) {
		if (indices[i].indexname == index_name) {
			indices.erase(indices.begin() + i);
			indexNum--;
		}
	}
}

void CatalogManager::CatalogManager::update(Table& tableinfor) {
	for (int i = 0; i < tableNum; i++) {
		if (tables[i].tablename == tableinfor.tablename) {
			tables[i].attriNum = tableinfor.attriNum;
			tables[i].blockNum = tableinfor.blockNum;
			tables[i].tupleLength = tableinfor.tupleLength;
			tables[i].attributes = tableinfor.attributes;
		}
	}
}

void CatalogManager::update(Index& index) {
	for (int i = 0; i< indexNum; i++) {
		if (indices[i].indexname == index.indexname) {
			indices[i].tablename = index.tablename;
			indices[i].column = index.column;
			indices[i].blockNum = index.blockNum;
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
			break;//found it
	}
	if (i >= indices.size()) return 0;
	else return 1;
}

bool CatalogManager::ExistIndex(std::string indexname) {
	int i;
	for (i = 0; i <indices.size(); i++) {
		if (indices[i].indexname == indexname)
			break;//found it
	}
	if (i >= indices.size()) return 0;
	else return 1;
}

Table CatalogManager::getTableInfo(std::string tablename) {
	int i;
	Table temp;
	for (i = 0; i<tableNum; i++) {
		if ((tables[i].tablename) == tablename) {

			return tables[i];
		}
	}
	return temp;
}

Index CatalogManager::getIndexInfo(std::string indexName) {
	int i;
	for (i = 0; i < tableNum; i++) {
		if (indices[i].indexname == indexName)
			break;//found it
	}
	if (i >= indexNum) {
		Index tmpt;
		return tmpt;//indicate that table information not found
	}
	return indices[i];
}

void CatalogManager::ShowCatalog() {
	ShowTableCatalog();
	ShowIndexCatalog();
}


int CatalogManager::GetColumnIndex(Table& table, Attribute &a)
{
	for (auto it = table.attributes.begin(); it != table.attributes.end(); ++it) {
		if (a.name == it->name)
			return 1;
	}
	return -1;
}

int CatalogManager::GetColumnAmount(Table& tableinfo) {
	return tableinfo.attributes.size();
}

//将item分解，形成一个tuple
void CatalogManager::splitDataItem(Table &t, Tuple &tuple, std::string item)
{
	int i = 0;
	std::string str;
	for (auto it = t.attributes.begin(); it != t.attributes.end(); ++it) {
		for (int j = i; j < i + it->length; j++)
			str += item[i];
		i += it->length;
		tuple.columns.push_back(str);
	}
}


