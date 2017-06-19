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

// class used to store the meta data of a BPlusTree
class MetaData{
public:
    MetaData(){
        max_children = 0;
        key_size = sizeof(key_t);
        value_size = sizeof(value_size);
        height = 0;
        inner_node_num = 0;
        leave_offset = 0;
    }
    
    int max_children;       // the order of the tree
    int key_size;           // size of key
    int value_size;         // size of value
    int height;             // the height of the tree
    int inner_node_num;     // numbers of inner nodes in the tree
    int leaf_node_num;      // numbers of leaf nodes in the tree
    off_t root_offset;        // where is root stored
    off_t leave_offset;       // where is the first leave stored
    off_t slot;               // the newest available place
}

// define inner node and leaf node
class inner_node_t{
    off_t parent;                   // offset of the parent of this node
    off_t next;                     // offset of the next sibling of the node
    off_t prev;                     // offset of the previous sibling of the node
    int children_num;               // how many children does this node have
    index_t children[TREE_ORDER];   // children array
}

class leaf_node_t{
    off_t parent;                   // offset of the parent of this node
    off_t next;                     // offset of the next sibling of the node
    off_t prev;                     // offset of the previous sibling of the node
    int record_num;                 // how many record does this node have
    record_t record[TREE_ORDER];    // children array
}


class BPlusTree{
public:
    BPlusTree(string directory);
private:
    MetaData meta;
}

