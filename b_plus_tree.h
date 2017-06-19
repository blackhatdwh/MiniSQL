// based on https://github.com/halfvim/minidb
#define TREE_ORDER 4
#define META_OFFSET 0
#define BLOCK_OFFEST META_OFFSET + sizeof(MetaData)
using namespace std;

// define key type and value type
struct key_t{
    string* key_content;
    key(){
        key_content = new string;
    }
};
typedef int value_t;

// define index type and record type
struct index_t{
    key_t key;
    off_t child;
};

struct record_t{
    key_t key;
    value_t value;
}

// define inner node and leaf node
class inner_node_t{
    off_t parent_;                   // offset of the parent of this node
    off_t next_;                     // offset of the next sibling of the node
    off_t prev_;                     // offset of the previous sibling of the node
    int children_num_;               // how many children does this node have
    index_t children_[TREE_ORDER];   // children array
}

class leaf_node_t{
    off_t parent_;                   // offset of the parent of this node
    off_t next_;                     // offset of the next sibling of the node
    off_t prev_;                     // offset of the previous sibling of the node
    int record_num_;                 // how many record does this node have
    record_t record_[TREE_ORDER];    // children array
}

// class used to store the meta data of a BPlusTree
class MetaData{
public:
    MetaData(){
        max_children_ = 0;
        key_size_ = sizeof(key_t);
        value_size_ = sizeof(value_size);
        height_ = 0;
        inner_node_num_ = 0;
        leave_offset_ = 0;
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
}



class BPlusTree{
public:
    BPlusTree(string directory);
     // Search for a record marked by $key, and store the data into $result. Return success or fail
    int Search(key_t key, value_t* result);

    // Insert a record whose key is $key and value is &value
    int Insert(key_t key, value_t value);

private:
    MetaData meta_;
}

