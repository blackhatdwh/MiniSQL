#include <iostream>
#include "IndexManager.h"
using namespace std;

IndexManager::IndexManager(){
    tree_ = nullptr;
    directory_ = "";
}

IndexManager::~IndexManager(){
    delete tree_;
}


bool IndexManager::CheckExist(string directory){
    ifstream check_stream(directory);
    if(check_stream.good()){
        return true;
    }
    else{
        return false;
    }
}

void IndexManager::Create(string directory){
    if(!CheckExist(directory)){
        // delete the previous tree
        delete tree_;
        // update the current index file directory
        directory_ = directory;
        // create a new b plus tree from empty
        tree_ = new BPlusTree(directory, false);
        // TODO
        // other component use for loop to insert record into this tree
    }
}

void IndexManager::Insert(string directory, char* key, int value){
    if(directory != directory_){
        if(CheckExist(directory)){
            LoadIndex(directory);
        }
    }
    m_key_t temp_key(key);
    value_t temp_value = value;
    tree_->Insert(temp_key, temp_value);
}

int IndexManager::Search(string directory, char* key){
    if(directory != directory_){
        if(!CheckExist(directory)){
            return -1;
        }
        else{
            LoadIndex(directory);
        }
    }
    m_key_t temp_key(key);
    int result = tree_->Search(temp_key);
    return result;
}

void IndexManager::Delete(string directory){
    remove(directory.c_str());
    if(directory == directory_){
        delete tree_;
        tree_ = nullptr;
    }
}

void IndexManager::LoadIndex(string directory){
    delete tree_;
    tree_ = new BPlusTree(directory, true);
    directory_ = directory;
}
