#include <iostream>
#include "b_plus_tree.h"
using namespace std;

IndexManager::IndexManager(string directory){
    directory_ = directory;
    CheckAlreadyExist();
    if(already_exist_){
        tree_ = new BPlusTree(directory_, true);
    }
    else{
        tree_ = new BPlusTree(directory_, false);
    }
}

IndexManager::~IndexManager(){
    delete tree_;
}

IndexManager::CheckAlreadyExist(){
    ifstream check_stream(directory_);
    if(check_stream.good()){
        already_exist_ = true;
    }
    else{
        already_exist_ = false;
    }
}

void IndexManager::Insert(char* key, int value){
    m_key_t temp_key(key);
    value_t temp_value = value;
    tree_->Insert(temp_key, temp_value);
}

int Search(char* key){
    m_key_t temp_key(key);
    value_t temp_value = value;
    int result = tree_->Search(temp_key, &temp_value);
    return result;
}
