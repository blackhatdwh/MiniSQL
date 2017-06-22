#include "b_plus_tree.h"
#include <cstring>
using namespace std;

record_t* SearchRecord(leaf_node_t* leaf, key_t key){
	return FirstNotLessThan(leaf->children_, 0, leaf->record_num_, key);
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

template <typename T>
bool ExistInArray(T* children_array, int begin, int end, key_t criterion){
    for(int i = begin; i < end; i++){
        if(children_array[i].key == criterion){
            return true;
        }
    }
    return false;
}
BPlusTree::BPlusTree(string directory, bool from_empty):directory_(directory){
    if(from_empty){
        Init();
    }
    //TODO
    else{

    }
}

// read the leaf where the record supposed to be attached
off_t GetSupposedLeaf(key_t key, leaf_node_t* record_parent, off_t* retrieve_record_grandparent_offset = nullptr){
    // find the offset of record's grandparent 
    off_t record_grandparent_offset = SearchIndex(key);
    // find the offset of record's parent based on the prevoius search
    off_t record_parent_offset = SearchLeaf(record_grandparent_offset, key);
    // read the parent of the record from disk
    Read(record_parent_offset, record_parent);
    if(record_parent_offset != nullptr){
        *retrieve_record_parent_offset = record_parent_offset;
    }
    return record_parent_offset;
}

void BPlusTree::Search(key_t key, value_t* result){
    leaf_node_t record_parent;
    GetSupposedLeaf(key, &record_parent);
    // check whether the record exists in the parent's child array
    record_t* record = SearchRecord(&record_parent, key);
    // found. Pass the value to $result
    if(record != nullptr){
        *result = record->value;
    }

    // not found. set $ersult to nullptr
    else{
        result = nullptr;
    }
}

void BPlusTree::Insert(key_t key, value_t value){
    leaf_node_t record_parent;
    off_t record_grandparent_offset;
    // GetSupposedLeaf also set $record_parent's value
    off_t record_parent_offset = GetSupposedLeaf(key, &record_parent, &record_grandparent_offset);
    // if the record already exists, return
    if(ExistInArray(&record_parent, key)){
        return;
    }
    // else, do the insertion operation
    // if full, split
    if(record_parent.children_num_ == TREE_ORDER){
        // create new leaf
        leaf_node_t new_leaf;
        CreateNode(record_parent_offset, &record_parent, &new_leaf);
        // where to split
        size_t split_point = record_parent.children_num_ / 2;
        // add to left part or right part
        bool place_right = (key > record_parent.record_[point].key);
        if(place_right){
            split_point++;
        }
        // copy the right part of the original leaf to the new leaf
        std::copy(record_parent.children_ + split_point, record_parent.children_+ record_parent.children_num_, new_leaf.children_);
        // update new leaf's children number
        new_leaf.children_num_ = record_parent.children_num_ - split_point;
        // update original leaf's children number
        record_parent.children_num_ = split_point;
        // insert record whether to left or to right
        if(place_right){
            InsertRecordNoSplit(&new_leaf, key, value);
        }
        else{
            InsertRecordNoSplit(&record_parent, key, value);
        }
        // save old and new leaves to hard disk
        Write(record_parent_offset, &record_parent);
        Write(record_parent.next_, &new_leaf);
        InsertKeyToIndex(record_grandparent_offset, new_leaf.children_[0].key, record_parent_offset, record_parent.next_);
    }
    // else, insert directly
    else{
        InsertRecordNoSplit(&record_parent, key, value);
        Write(offset, &record_parent);
    }
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




void BPlusTree::InsertKeyToIndex(){

}

void BPlusTree::InsertKeyToIndexNoSplit(){

}

void BPlusTree::InsertRecordNoSplit(leaf_node_t* record_parent, key_t key, value_t value){

}

template<typename T>
void BPlusTree::CreateNode(off_t original_offset, T* original_node, T* new_node) {
    // insertion like linked list
    new_node->parent_ = original_node->parent_;
    new_node->next_ = original_node->next_;
    new_node->prev_ = original_offset;
    original_node->next_ = alloc(new_node);
    // update the old next node if it exists
    if(new_node->next_ != 0){
        T old_next;
        // retrieve the old next node
        Read(new_node->next, &old_next);
        // modify its prev_
        old_next.prev = original_node->next_;
        // place it back
        Write(new_node->next_, &old_next);
    }
    // update the meta data
    Write(META_OFFSET, &meta_);
}

