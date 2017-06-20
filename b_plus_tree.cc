#include "b_plus_tree.h"
using namespace std;

BPlusTree::BPlusTree(string directory, bool from_empty):directory_(directory){
    if(from_empty){
        Init();
    }
}

void BPlusTree::Search(key_t key, value_t* result){

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
    meta_.root_offset = alloc(&root);

    // create empty leaf
    leaf_node_t leaf;
    leaf.parent_ = meta_.root_offset_;
    root.children_[0].child = alloc(&leaf);
    meta_.leave_offset_ = root.children_[0].child;

    //save
    Write(OFFSET_META, &meta_);
    Write(meta_.root_offset_, &root);
    Write(root.children_[0].child, &leaf);
}

off_t BPlusTree::SearchIndex(){

}

off_t BPlusTree::SearchLeaf(){

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
void CreateNode(){

}
