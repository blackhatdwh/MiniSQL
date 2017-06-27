#include "RecordManager.h"
#include "BufferManager.h"
#include "CatalogManager.h"

#include <fstream>
#include <string>
#include <vector>
using namespace std;

extern BufferManager BM;
extern CatalogManager cat;

void MeetCondition(Attribute &attr, condition &cond)
{
	if (cond.ope1 == "=") {

	}
	else if (cond.ope1 == "<") {

	}
	else if (cond.ope1 == ">") {

	}
	else if (cond.ope1 == "<=") {

	}
	else if (cond.ope1 == ">=") {

	}
	else if (cond.ope1 == "<") {

	}
}

void RecordManager::Select(Attribute &attr, Table &t, condition &cond, std::vector<Tuple> &tuples)
{
	// open the file
	string filename = t.tablename + ".data";
	ifstream fin(t.tablename);
	if (!fin) {
		cerr << "Can't open " << filename << endl;
	}

	vector<Tuple> tmp;
	
	
}


void RecordManager::Delete()
{

}

void RecordManager::Insert()
{

}

