#ifndef INDEXMANAGER_H
#define INDEXMANAGER_H
#include "b_plus_tree.h"
#include <cstdio>
#include <vector>

class IndexManager{
public:
    IndexManager();
    ~IndexManager();
    bool CheckExist(string table_name, int col_num);      // check if the index exists
    void Create(string table_name, int col_num);          // create a index called directory
    void Insert(string table_name);        // insert a key-value pair into index directory
    int Search(string table_name, int col_num, char* key);        // search for the value of key in index directory
    void DeleteIndex(string table_name, int col_num);      // delete an index directory
    void DeleteKey(string table_name)
private:
    bool CheckExist(string directory);      // check if the index exists
    void LoadIndex(string directory);
    string GenerateIndexDirectory(string table_name, int col_num);
    vector<string> IndexColumnOfTable(string table_name);
    BPlusTree* tree_;
    string directory_;
};

#endif /* !INDEXMANAGER_H */
