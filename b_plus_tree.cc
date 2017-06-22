#include "b_plus_tree.h"
#include <cstring>
using namespace std;

record_t* SearchRecord(leaf_node_t* leaf, key_t key){
	return FirstNotLessThan(leaf->record_, 0, leaf->record_num_, key);
}

template <typename T>
T* FirstBiggerThan(T* children_array, int begin, int end, key_t criterion){
    for(int i = begin; i < end; i++){
        if(children_array[i].key > criterion){
            return &(children_array[i]);
        }
    }
    return nullptr;
}

template <typename T>
T* FirstNotLessThan(T* children_array, int begin, int end, key_t criterion){
    for(int i = begin; i < end; i++){
        if(children_array[i].key >= criterion){
            return &(children_array[i]);
        }
    }
    return nullptr;
}

BPlusTree::BPlusTree(string directory, bool from_empty):directory_(directory){
    if(from_empty){
        Init();
    }
    //TODO
    else{

    }
}

void BPlusTree::Search(key_t key, value_t* result){
    off_t record_grandparent_offset = SearchIndex(key);
	off_t record_parent_offset = SearchLeaf(record_grandparent_offset, key);
    leaf_node_t record_parent;
    Read(record_parent_offset, &record_parent);
    record_t* record = SearchRecord(&record_parent, key);
    if(record != nullptr){
        *result = record->value;
    }
    else{
        result = nullptr;
    }
}

void BPlusTree::Insert(key_t key, value_t value){

}

void BPlusTree::Init(){
    // create a file in the given directory
    ofstream index_file;
    index_file.open(directory_);
    index_file.close();

    // create root
    inner_node_t root;
    meta_.root_offset_ = alloc(&root);

    // create empty leaf
    leaf_node_t leaf;
    leaf.parent_ = meta_.root_offset_;
    root.children_[0].child = alloc(&leaf);
    meta_.leave_offset_ = root.children_[0].child;

    //save
    Write(META_OFFSET, &meta_);
    Write(meta_.root_offset_, &root);
    Write(root.children_[0].child, &leaf);
}

off_t BPlusTree::SearchIndex(key_t key){
    off_t record_grandparent_offset = meta_.root_offset_;
    int height = meta_.height_;
    // if height > 1, then we need to go down through more than one index
    while(height > 1){
        inner_node_t step;
        Read(record_grandparent_offset, &step);
        index_t* next_step = FirstBiggerThan(step.children_, 0, step.children_num_, key);
        record_grandparent_offset = next_step->child;
        height--;
    }
    return record_grandparent_offset; 
}

off_t BPlusTree::SearchLeaf(off_t record_grandparent_offset, key_t key){
    inner_node_t record_grandparent;
    Read(record_grandparent_offset, &record_grandparent);
    index_t* index_to_record_parent = FirstBiggerThan(record_grandparent.children_, 0, record_grandparent.children_num_, key);
    return index_to_record_parent->child;
}


void BPlusTree::BorrowKey(bool from_right, inner_node_t* borrower, off_t offset){

}

void BPlusTree::BorrowKey(bool from_right, leaf_node_t* borrower, off_t offset){

}

void BPlusTree::ChangeNodeParent(){

}

void BPlusTree::InsertInnerNode(){

}

void BPlusTree::InsertInnerNodeNoSplit(){

}

void BPlusTree::InsertRecord(){

}

template<typename T>
void BPlusTree::CreateNode() {

}

template<typename T>
void CreateNode(){

}

