// based on  https://github.com/zcbenz/BPlusTree
#include "b_plus_tree.h"
#include <cstring>
using namespace std;


template <typename T>
T* FirstBiggerThan(T* children_array, int begin, int end, m_key_t criterion, int* sequence = 0){
    for(int i = begin; i < end; i++){
        if(children_array[i].key > criterion){
            *sequence = i;
            return &(children_array[i]);
        }
    }
    return &children_array[end - 1];
}

template <typename T>
T* FirstNotLessThan(T* children_array, int begin, int end, m_key_t criterion){
    for(int i = begin; i < end; i++){
        if(children_array[i].key >= criterion){
            return &(children_array[i]);
        }
    }
    return &children_array[end - 1];
}

record_t* SearchRecord(leaf_node_t* leaf, m_key_t key){
	return FirstNotLessThan(leaf->children_, 0, leaf->children_num_, key);
}

template <typename T>
bool ExistInArray(T* children_array, int begin, int end, m_key_t criterion){
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
off_t BPlusTree::GetSupposedLeaf(m_key_t key, leaf_node_t* record_parent, off_t* retrieve_record_grandparent_offset){
    // find the offset of record's grandparent 
    off_t record_grandparent_offset = SearchIndex(key);
    // find the offset of record's parent based on the prevoius search
    off_t record_parent_offset = SearchLeaf(record_grandparent_offset, key);
    // read the parent of the record from disk
    Read(record_parent_offset, record_parent);
    if(retrieve_record_grandparent_offset != nullptr){
        *retrieve_record_grandparent_offset = record_parent_offset;
    }
    return record_parent_offset;
}

void BPlusTree::Search(m_key_t key, value_t* result){
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

void BPlusTree::Insert(m_key_t key, value_t value){
    leaf_node_t record_parent;
    off_t record_grandparent_offset;
    // GetSupposedLeaf also set $record_parent's value
    off_t record_parent_offset = GetSupposedLeaf(key, &record_parent, &record_grandparent_offset);
    // if the record already exists, return
    if(ExistInArray(record_parent.children_, 0, record_parent.children_num_, key)){
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
        bool place_right = (key > record_parent.children_[split_point].key);
        if(place_right){
            split_point++;
        }
        // copy the right part of the original leaf to the new leaf
        //std::copy(record_parent.children_ + split_point, record_parent.children_+ record_parent.children_num_, new_leaf.children_);
        for(int i = split_point; i < record_parent.children_num_; i++){
            new_leaf.children_[i - split_point] = record_parent.children_[i];
        }
        // update new leaf's children number
        new_leaf.children_num_ = record_parent.children_num_ - split_point;
        // update original leaf's children number
        record_parent.children_num_ = split_point;
        // insert record whether to left or to right
        if(place_right){
            InsertRecordNoSplit(new_leaf, key, value);
        }
        else{
            InsertRecordNoSplit(record_parent, key, value);
        }
        // save old and new leaves to hard disk
        Write(record_parent_offset, &record_parent);
        Write(record_parent.next_, &new_leaf);
        InsertKeyToIndex(record_grandparent_offset, new_leaf.children_[0].key, record_parent_offset, record_parent.next_);
    }
    // else, insert directly
    else{
        InsertRecordNoSplit(record_parent, key, value);
        Write(record_parent_offset, &record_parent);
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

off_t BPlusTree::SearchIndex(m_key_t key){
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

off_t BPlusTree::SearchLeaf(off_t record_grandparent_offset, m_key_t key){
    inner_node_t record_grandparent;
    Read(record_grandparent_offset, &record_grandparent);
    index_t* index_to_record_parent = FirstBiggerThan(record_grandparent.children_, 0, record_grandparent.children_num_, key);
    return index_to_record_parent->child;
}




void BPlusTree::InsertKeyToIndex(off_t offset, m_key_t key, off_t old_node, off_t after){
    // if root has to split, create a new root
    if(offset == 0){
        // trivial init work
        inner_node_t root;
        meta_.root_offset_ = alloc(&root);
        meta_.height_++;
        // insert the two children
        root.children_num_ = 2;
        root.children_[0].key = key;
        root.children_[0].child = old_node;
        root.children_[1].child = after;
        // store the new data into hard disk
        Write(META_OFFSET, &meta_);
        Write(meta_.root_offset_, &root);
        // update children's parent
        SetNodeChildParent(&root, meta_.root_offset_);
        return;
    }

    inner_node_t new_node_parent;
    Read(offset, &new_node_parent);

    // if full, split
    if(new_node_parent.children_num_ == TREE_ORDER){
        // create new node
        inner_node_t new_node;
        CreateNode(offset, &new_node_parent, &new_node);
        int split_point = (new_node_parent.children_num_ - 1) / 2;
        // decide whether to put the new node in the right part or the left part
        bool place_right = (key > new_node_parent.children_[split_point].key);
        if(place_right){
            split_point++;
        }
        // black magic begin
        if(place_right && (new_node_parent.children_[split_point].key > key)){
            split_point--;
        }
        m_key_t middle_key = new_node_parent.children_[split_point].key;
        // black magic end
        // copy backward to spare space for the new node
        for(int i = split_point + 1; i < new_node_parent.children_num_; i++){
            new_node.children_[i - split_point - 1] = new_node_parent.children_[i];
        }
        new_node.children_num_ = new_node_parent.children_num_ - split_point - 1;
        new_node_parent.children_num_ = split_point + 1;
        // deploy the new key
        if(place_right){
            InsertKeyToIndexNoSplit(new_node, key, after);
        }
        else{
            InsertKeyToIndexNoSplit(new_node_parent, key, after);
        }
        // save to disk
        Write(offset, &new_node_parent);
        Write(new_node_parent.next_, &new_node);
        SetNodeChildParent(&new_node, new_node_parent.next_);
        InsertKeyToIndex(new_node_parent.parent_, middle_key, offset, new_node_parent.next_);
    }
    // else, directly insert without split
    else{
        InsertKeyToIndexNoSplit(new_node_parent, key, after);
        Write(offset, &new_node_parent);
    }
}

void BPlusTree::InsertKeyToIndexNoSplit(inner_node_t node, m_key_t key, off_t value){
    int position_num;
    index_t* position = FirstBiggerThan(node.children_, 0, node.children_num_, key, &position_num);
    // move children behind $position one step backward to spare one space for the new child
    for(int i = node.children_num_ - 1; i >= position_num; i--){
        node.children_[i + 1] = node.children_[i];
    }
    // do the inserting work
    position->key = key;
    position->child = (position + 1)->child;
    (position + 1)->child = value;
    node.children_num_++;
}

void BPlusTree::InsertRecordNoSplit(leaf_node_t leaf, m_key_t key, value_t value){
    int position_num;
    record_t* position = FirstBiggerThan(leaf.children_, 0, leaf.children_num_, key, &position_num);
    // move children behind $position one step backward to spare one space for the new child
    for(int i = leaf.children_num_ - 1; i >= position_num; i--){
        leaf.children_[i + 1] = leaf.children_[i];
    }
    position->key = key;
    position->value = value;
    leaf.children_num_++;
}

void BPlusTree::SetNodeChildParent(inner_node_t* node, off_t self_offset){
    inner_node_t child_temp;
    // update child of $node 's parent one by one
    for(int i = 0; i < node->children_num_; i++){
        // read in, modify, write back
        Read((node->children_)[i].child, &child_temp);
        child_temp.parent_ = self_offset;
        Write((node->children_)[i].child, &child_temp);
    }
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
        Read(new_node->next_, &old_next);
        // modify its prev_
        old_next.prev_ = original_node->next_;
        // place it back
        Write(new_node->next_, &old_next);
    }
    // update the meta data
    Write(META_OFFSET, &meta_);
}

