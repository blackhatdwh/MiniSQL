#ifndef INDEXMANAGER_H
#define INDEXMANAGER_H
#include "b_plus_tree.h"
#include <cstdio>

class IndexManager{
public:
    IndexManager();
    ~IndexManager();
    bool CheckExist(string directory);      // check if the index exists
    void Create(string directory);          // create a index called directory
    void Insert(string directory, char* key, int value);        // insert a key-value pair into index directory
    int Search(string directory, char* key);        // search for the value of key in index directory
    void Delete(string directory);      // delete an index directory
private:
    void LoadIndex(string directory);
    BPlusTree* tree_;
    string directory_;
};

#endif /* !INDEXMANAGER_H */
