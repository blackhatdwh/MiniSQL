#include "b_plus_tree.h"

using namespace std;

BPlusTree::BPlusTree(string directory):directory_(directory){

}

void BPlusTree::Search(key_t key, value_t* result){

}

void BPlusTree::Insert(key_t key, value_t value){

}

void BPlusTree::Init(){

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
