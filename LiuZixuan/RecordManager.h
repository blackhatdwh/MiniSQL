#ifndef __RECORDMANAGER_H__
#define __RECORDMANAGER_H__

#include "BufferManager.h"
#include "RecordManager.h"
#include "CatalogManager.h"
#include <iostream>
#include <vector>
#include <string>
#define EMPTY '@'
#define END '#'

extern BufferManager buf;
extern CatalogManager cam;

struct SQLCondition {
	int column;
	int ope;
	std::string value;
};

class RecordManager {
public:
	void Select(int column, Table &t, SQLCondition &cond, std::vector<Tuple> &tuples);
	void Select_Equal(Table &t, int column_to_compare, std::string value, std::vector<Tuple> &tuples);
	void Select(Table &t, std::vector<SQLCondition> &cond, std::vector<Tuple> &tuples);
	void Insert(Tuple &tuple, Table &t);
	void Delete(Table &t, std::vector<SQLCondition> &cond, std::vector<Tuple> &tuple);

	void Delete();
	void Insert();
	friend class IndexManeger;
	friend class BPlusTree;
	friend class CatalogManager;
	friend class RecordManager;
	friend class BufferManager;
private:

};

#endif

