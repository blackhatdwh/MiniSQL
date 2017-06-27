#ifndef __RECORDMANAGER_H__
#define __RECORDMANAGER_H__

#include "BufferManager.h"
#include "CatalogManager.h"
#include "RecordManager.h"
#include <iostream>
#include <vector>

extern BufferManager buf;
extern CatalogManager cat;

struct condition
{
	std::string ope1;
	std::string val1;
	std::string ope2;
	std::string val2;
	Attribute a;
};


class RecordManager {
public:
	void Select(Attribute &attr, Table &t, condition &cond, std::vector<Tuple> &tuples);
	void Delete();
	void Insert();
private:

};

#endif

