#ifndef INDEXMANAGER_H
#define INDEXMANAGER_H
#include "b_plus_tree.h"

class IndexManager(){
public:
    IndexManager(string directory);
    ~IndexManager();
    void Insert(char* key, int value);
    int Search(char* key);
private:
    void CheckAlreadyExist();

    BPlusTree* tree_;
    string directory_;
    bool already_exist_;
}

#endif /* !INDEXMANAGER_H */
