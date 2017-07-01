#include "RecordManager.h"
#include "BufferManager.h"
#include "CatalogManager.h"
#include "IndexManager.h"

#include <sstream>
#include <fstream>
#include <string>
#include <vector>
using namespace std;

extern BufferManager bfm;
extern CatalogManager cam;
extern RecordManager rem;
extern IndexManager idm;

enum{Equal, Inequal, Less, Greater, Nogreater, Noless};

inline string ReadData(char *s, int start, int length)
{
	char tmp[500];
	stringstream S(s+start);
	S.read(tmp, length);

}

inline void DeleteData(char *s, int start, int length)
{
	for (int i = start; i < start + length; i++) {
		s[i] = EMPTY;
	}
}

bool MatchCondition(Tuple &tuple, std::vector<Condition> &cond, int type)
{
	for (int i = 0; i < cond.size(); ++i) {
		string v1 = tuple.columns[cond[i].column];
		string v2 = cond[i].value;
		double a1 = atof(v1.c_str());
		double a2 = atof(v2.c_str());
		switch (cond[i].ope) {
			case Equal:
				if (v1 != v2)
					return false;
			case Inequal:
				if (v1 == v2)
					return false;
			case Less:
				if (type == CHAR)
					if (v1 >= v2)
						return false;
					else
						if (a1 >= a2)
							return false;
			case Greater:
				if (type == CHAR)
					if (v1 <= v2)
						return false;
				else
					if (a1 <= a2)
						return false;
			case Noless:
				if (type == CHAR)
					if (v1 < v2)
						return false;
				else
					if (a1 < a2)
						return false;
			case Nogreater:
				if (type == CHAR)
					if (v1 > v2)
						return false;
				else
					if (a1 > a2)
						return false;
			default:
				return false;
		}
	}
	return true;
}


void RecordManager::Select(Table &t, std::vector<Condition> &cond, std::vector<Tuple> &tuples)
{
	int BufferID;
	for (int i = 0; i < t.blockNum; i++) {
		int start = 0;
		BufferID = buf.Load(t.tablename, i);
		Tuple temp_tuple;
		string str = ReadData(buf.bufferblocks[BufferID].contents, start, t.tupleLength);
		while (str[0] != '#' && start <= BLOCKSIZE-t.tupleLength) {
			if (str[0] != '@') {
				cam.SplitDataItem(t, temp_tuple, str);
				if (MatchCondition(temp_tuple, cond, t.attributes[cond[i].column].type))
					tuples.push_back(temp_tuple);
			}
			start += t.tupleLength;
		}
	}
}


void RecordManager::Insert(Tuple &tuple, Table &t)
{
	BufferPosition bp = bfm.GetInsertPosition(t);
	string str;
	//拼接
	for (int i = 0; i < tuple.columns.size(); ++i) {
		str += tuple.columns[i];
	}
	char *p = bfm.bufferblocks[bp.BufferID].contents() + bp.offset;
	int i;
	for (i = 0; i < str.size(); i++) {
		p[i] = str[i];
	}
	p[i] = EMPTY;

}

void RecordManager::Delete(Table &t, std::vector<Condition> &cond, std::vector<Tuple> &tuples)
{
	int BufferID;
	for (int i = 0; i < t.blockNum; i++) {
		int start = 0;
		BufferID = buf.Load(t.tablename, i);
		Tuple temp_tuple;
		string str = ReadData(buf.bufferblocks[BufferID].contents, start, t.tupleLength);
		while (str[0] != END && start <= BLOCKSIZE - t.tupleLength) {
			if (str[0] != EMPTY) {
				cam.SplitDataItem(t, temp_tuple, str);
				if (MatchCondition(temp_tuple, cond, t.attributes[cond[i].column].type)) {
					tuples.push_back(temp_tuple);
					DeleteData(bfm.bufferblocks[BufferID].contents, start, t.tupleLength);
				}	
			}
			start += t.tupleLength;
		}
	}
}
