#ifndef INDEXMANAGER_H
#define INDEXMANAGER_H
#include "b_plus_tree.h"
#include <cstdio>

class IndexManager{
public:
    IndexManager();
    ~IndexManager();
    bool CheckExist(string directory);
    void Create(string directory);
    void Insert(string directory, char* key, int value);
    int Search(string directory, char* key);
    void Delete(string directory);
private:
    void LoadIndex(string directory);
    BPlusTree* tree_;
    string directory_;
};

#endif /* !INDEXMANAGER_H */
