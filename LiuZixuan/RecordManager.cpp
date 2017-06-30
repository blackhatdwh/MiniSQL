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

inline void read_str(string str, char *s, int start, int length)
{
	for (int i = start; i < start + length && s[i]; i++) {
		str[i - start] = s[i];
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
}

void RecordManager::Select(Table &t, std::vector<Condition> &cond, std::vector<Tuple> &tuples)
{
	for (int i = 0; i < cond.size(); ++i) {
		if (cam.ExistIndex(t.tablename, cond[i].column)) {
			int p = idm.Search(t.tablename, cond[i].column, cond[i].value.c_str());
			int BufferID = bfm.Load(t.tablename, p / BLOCKSIZE);
			Tuple temp_tuple;
			string str = "#";
			p = p % BLOCKSIZE;
			read_str(str, bfm.bufferblocks[BufferID].contents, p, t.tupleLength);
			cam.SplitDataItem(t, temp_tuple, str);
			tuples.push_back(temp_tuple);
			return;
		}
	}

	int start = 0;
	int BufferID;
	for (int i = 0; i < t.blockNum; i++) {
		BufferID = buf.Load(t.tablename, i);
		Tuple temp_tuple;
		string str = "#";
		read_str(str, buf.bufferblocks[BufferID].contents, start, t.tupleLength);
		while (str[0] != '#') {
			cam.SplitDataItem(t, temp_tuple, str);
			start += t.tupleLength;
			if (MatchCondition(temp_tuple, cond, t.attributes[cond[i].column].type))
				tuples.push_back(temp_tuple);
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
	for (int i = 0; i < cond.size(); ++i) {
		if (cam.ExistIndex(t.tablename, cond[i].column)) {
			int p = idm.Search(t.tablename, cond[i].column, cond[i].value.c_str());
			int BufferID = bfm.Load(t.tablename, p / BLOCKSIZE);
			Tuple temp_tuple;
			string str = "#";
			p = p % BLOCKSIZE;
			read_str(str, bfm.bufferblocks[BufferID].contents, p, t.tupleLength);
			cam.SplitDataItem(t, temp_tuple, str);
			tuples.push_back(temp_tuple);
			return;
		}
	}

	int start = 0;
	int BufferID;
	for (int i = 0; i < t.blockNum; i++) {
		BufferID = buf.Load(t.tablename, i);
		Tuple temp_tuple;
		string str = "#";
		read_str(str, buf.bufferblocks[BufferID].contents, start, t.tupleLength);
		while (str[0] != '#') {
			cam.SplitDataItem(t, temp_tuple, str);
			start += t.tupleLength;
			if (MatchCondition(temp_tuple, cond, t.attributes[cond[i].column].type))
				tuples.push_back(temp_tuple);
		}
	}
}

//void RecordManager::Select(int column, Table &t, condition &cond, std::vector<Tuple> &tuples)
//{
//	// open the file
//	string filename = t.tablename + ".data";
//	ifstream fin(t.tablename);
//	if (!fin) {
//		cerr << "Can't open " << filename << endl;
//		return;
//	}
//	
//	if (cam.ExistIndex(t.tablename, column)) {
//		//使用索引
//		if (cond.ope1 == "==") {
//			//根据cond.val进行匹配，得到其地址
//		}
//		else if (cond.ope1 == "<") {
//			if (cond.ope2 == ">") {
//				//区间查找
//			}
//			else if (cond.ope2 == ">=") {
//
//			}
//		}
//		else if (cond.ope1 == ">") {
//			if (cond.ope2 == "<") {
//
//			}
//			else if (cond.ope2 == "<=") {
//
//			}
//		}
//		else if (cond.ope1 == "<=") {
//			if (cond.ope2 == "<") {
//
//			}
//			else if (cond.ope2 == "<=") {
//
//			}
//		}
//		else if (cond.ope1 == ">=") {
//			if (cond.ope2 == "<") {
//
//			}
//			else if (cond.ope2 == "<=") {
//
//			}
//		}
//	}
//	else {
//
//	}
//
//	fin.close();
//	
//}
//

//
//void RecordManager::Select_Equal(Table &t, int column_to_compare, std::string value, std::vector<Tuple> &tuples)
//{
//	int SingleLength = t.tupleLength + 1;
//	if (cam.ExistIndex(t.tablename, column_to_compare)) {
//		int p = idm.Search(t.tablename, column_to_compare, value.c_str());
//		int BufferID = bfm.Load(t.tablename, p / (t.tupleLength + 1));
//		Tuple temp_tuple;
//		string str = "#";
//		p = p % (t.tupleLength + 1);
//		read_str(str, bfm.bufferblocks[BufferID].contents, p, SingleLength);
//		cam.SplitDataItem(t, temp_tuple, str);
//		tuples.push_back(temp_tuple);
//	}
//	else {
//		int start = 0;
//		int BufferID;
//		for (int i = 0; i < t.blockNum; i++) {
//			BufferID = buf.Load(t.tablename, i);
//			Tuple temp_tuple;
//			string str = "#";
//			read_str(str, buf.bufferblocks[BufferID].contents, start, SingleLength);
//			while (str[0] != '#') {
//				cam.SplitDataItem(t, temp_tuple, str);
//				start += SingleLength;
//				if (temp_tuple.columns[column_to_compare] == value)
//					tuples.push_back(temp_tuple);
//			}
//		}
//	}
//}
//
//void 
//
//void RecordManager::Insert()
//{
//
//}

