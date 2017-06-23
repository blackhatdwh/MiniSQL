// based on  https://github.com/zcbenz/BPlusTree

#include <iostream>
#include <fstream>
#include <cstring>
#define TREE_ORDER 4
#define META_OFFSET 0
#define BLOCK_OFFEST META_OFFSET + sizeof(MetaData)
using namespace std;

// define key type and value type
struct m_key_t{
    char key_content[16];
    m_key_t(char* content = ""){
        sprintf(key_content, "%s", content);
    }
    bool operator>(const m_key_t& other){
        int difference = strcmp(this->key_content, other.key_content);
        if(difference > 0){
            return true;
        }
        return false;
    }
    bool operator>=(const m_key_t& other){
        int difference = strcmp(this->key_content, other.key_content);
        if(difference >= 0){
            return true;
        }
        return false;
    }
    bool operator==(const m_key_t& other){
        int difference = strcmp(this->key_content, other.key_content);
        if(difference == 0){
            return true;
        }
        return false;
    }
};
typedef int value_t;

// define index type and record type
struct index_t{
    m_key_t key;
    off_t child;
};

struct record_t{
    m_key_t key;
    value_t value;
};

// define inner node and leaf node
struct inner_node_t{
    inner_node_t():parent_(0), next_(0), prev_(0), children_num_(0){}
    off_t parent_;                   // offset of the parent of this node
    off_t next_;                     // offset of the next sibling of the node
    off_t prev_;                     // offset of the previous sibling of the node
    int children_num_;               // how many children does this node have
    index_t children_[TREE_ORDER];   // children array
};

struct leaf_node_t{
    leaf_node_t():parent_(0), next_(0), prev_(0), children_num_(0){}
    off_t parent_;                   // offset of the parent of this node
    off_t next_;                     // offset of the next sibling of the node
    off_t prev_;                     // offset of the previous sibling of the node
    int children_num_;                 // how many record does this node have
    record_t children_[TREE_ORDER];    // children array
};

// class used to store the meta data of a BPlusTree
struct MetaData{
    MetaData(){
        max_children_ = TREE_ORDER;
        key_size_ = sizeof(m_key_t);
        value_size_ = sizeof(value_t);
        height_ = 1;
        inner_node_num_ = 0;
        leave_offset_ = 0;
        slot_ = BLOCK_OFFEST;
    }
    
    int max_children_;       // the order of the tree
    int key_size_;           // size of key
    int value_size_;         // size of value
    int height_;             // the height of the tree
    int inner_node_num_;     // numbers of inner nodes in the tree
    int leaf_node_num_;      // numbers of leaf nodes in the tree
    off_t root_offset_;        // where is root stored
    off_t leave_offset_;       // where is the first leave stored
    off_t slot_;               // the newest available place
};



class BPlusTree{
public:
    BPlusTree(string directory, bool from_empty);
     // Search for a record marked by $key, and store the data into $result. Return success or fail
    void Search(m_key_t key, value_t* result);

    // Insert a record whose key is $key and value is &value
    void Insert(m_key_t key, value_t value);

private:
    /* member variable */
    string directory_;
    FILE* fp_;
    MetaData meta_;

    // initialize a b plus tree
    void Init();
    
	// get the position of a record where it should be. Return it's imaginary parent's offset, and set value for $record_parent and $retrieve_record_grandparent_offset
	off_t GetSupposedLeaf(m_key_t key, leaf_node_t* record_parent, off_t* retrieve_record_grandparent_offset = nullptr);

    // search through the layer and return the node on the lowest layer which the wanted record attached to
    off_t SearchIndex(m_key_t key);

    // search through the children of the previous found node and find out our wanted record
    off_t SearchLeaf(off_t index, m_key_t key);

    void InsertKeyToIndex(off_t offset, m_key_t key, off_t old, off_t after);
    void InsertKeyToIndexNoSplit(inner_node_t& node, m_key_t key, off_t value);
    void InsertRecordNoSplit(leaf_node_t& record_parent, m_key_t key, value_t value);

    void SetNodeChildParent(inner_node_t* node, off_t self_offset);

    template<typename T>
    void CreateNode(off_t original_offset, T* original_node, T* new_node);


    /* infrastructural functions */
    // allocate a space for a node
    off_t alloc(int size){
        off_t slot = meta_.slot_;
        meta_.slot_ += size;
        return slot;
    }
    off_t alloc(inner_node_t* node){
        meta_.inner_node_num_++;
        node->children_num_ = 1;
        return alloc(sizeof(inner_node_t));
    }
    off_t alloc(leaf_node_t* node){
        meta_.leaf_node_num_++;
        node->children_num_ = 0;
        return alloc(sizeof(leaf_node_t));
    }

    // read and write from hard disk
    // read a block which locates at offset from the b_plus_tree file
    template<typename T>
    void Read(off_t offset, T* block){
        ifstream file;
        file.open(directory_, ios::in | ios::binary);
        file.seekg(offset, ios_base::beg);
        file.read(reinterpret_cast<char*>(block), sizeof(T));
        file.close();
    }

    // write a block which locates at offset to the b_plus_tree file
    template<typename T>
    void Write(off_t offset, T* block){
        ofstream file;
        file.open(directory_, ios::in | ios::binary);
        file.seekp(offset, ios_base::beg);
        file.write(reinterpret_cast<char*>(block), sizeof(T));
        file.close();
    }



};

