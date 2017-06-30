# IndexManager 设计说明
## 总体设计
### 功能简介
本模块负责MiniSQL的索引管理，用于实现以下功能：
- 表的索引的建立和删除
- 当表中数据发生更新时，同步更新该表涉及到的索引
- 在查找操作时通过索引为其他模块提供快速的记录寻址

### 具体功能
1. 当API要求创建一张新的表时，本模块为这张表的主键创建索引。此时索引为空。
2. 当API要求为一张已存在的表的某一列创建索引时，本模块为此表的该列创建索引，并将该列已存在的数据添加进索引。
3. 当API要求删除某一张表时，本模块将所有该表上的索引文件删除。
4. 当API要求删除某张表的某一列的索引时，本模块将该表该列的索引文件删除。
5. 当向某张表中插入记录时，本模块自动为该表上的所有索引添加这条新记录的索引记录。
6. 当向某张表中删除记录时，本模块自动为该表上的所有索引删除这条记录的索引记录。
7. 当需要进行查找操作时，本模块根据其他模块传入的信息在索引中进行查找，如果查找到相关记录则返回文件偏移。

### 实现方法
- 本模块的索引通过B+树实现
- 本模块的索引以文件的形式储存在硬盘中
- 所有索引文件都储存在程序当前目录下的 .index/目录中。./.index目录下有多个文件夹，每个文件夹中存放一张表的所有索引。比如假设本数据库中有如下表：
```
CREATE TABLE student (
    ID varchar(10),
    Name varchar(20),
    Gender varchar(1),
);
CREATE TABLE teacher (
    ID varchar(10),
    Name varchar(20),
    Gender varchar(1),
);
```
其中表student的ID和Name列上建立有索引，表teacher的ID列上建立有索引。则索引的文件目录结构如下：
```
.
├── .index
│   ├── student
│   │   ├── 0
│   │   └── 1
│   └── teacher
│       └── 0
└── MiniSQL.exe
```
其中student目录下的0为student表ID列的索引文件，1为student表Name列的索引文件。容易看出索引文件名以该列在表中的排序为文件名。
- 本模块在同一时间最多只加载一个索引文件。当需要尽心操作的索引并非当前加载的索引时，卸载当前索引，并加载需要的索引。这种方法效率较低，但实现简单。

##原理
### 数据结构
本模块主要使用的数据结构为B+树。B+树的实现参考
https://github.com/zcbenz/BPlusTree
#### 结构与类
B+树用于存储键值对。关于键值对存在如下结构和类：
- 键的结构 m_key_t
```
struct m_key_t{
    char key_content[32];
    bool operator>(const my_key_t& other);
    bool operator>=(const my_key_t& other);
    bool operator<(const my_key_t& other);
    bool operator<=(const my_key_t& other);
    bool operator==(const my_key_t& other);
};
```
实际上就时一个char的数组。但将其封装为结构，可以方便地进行运算符重载。因为在B+树的操作过程中需要常常会用到键的大小比较，而将比较大小的运算符重载，而不用较为麻烦的strcmp()，可以提高编码效率。
<br>之所以在其前面加上m_的前缀，是因为key_t似乎是g++的内建类型，会发生冲突。

- 值的结构value_t
```
typedef int value_t;
```
value_t就是int。在本程序中，value_t用来表示某条记录在文件中的偏移。将其写成value_t的好处在于方便辨认，并且方便替换。

B+树中有两种节点，内部节点和叶子节点。关于节点存在一下结构和类：
![B+](~/Desktop/DeepinScreenshot20170622201446.png)
- 内部节点 inner_node_t
<br>上图中黄色和绿色的节点即内部节点。内部节点以其他的内部节点或叶子节点为子节点。
```
struct inner_node_t{
    off_t parent_;    // 父节点的偏移
    off_t next_;     // 下一个节点的偏移
    off_t prev_;     // 上一个节点的偏移
    int children_num_;    // 现有的孩子数
    index_t children_[TREE_ORDER];    // 孩子们
}
```
一棵树中的每一个节点都储存在同一个文件中，而每一个节点数据在文件中又有着不同的偏移值。通过不同的偏移值，可以分辨不同的节点。内部节点的父节点，前后兄弟节点就是通过这种方式储存的。

- 叶子节点 leaf_node_t
<br>上图中蓝色节点即为叶子节点。叶子节点没有子节点，仅仅连接着记录，即图中红色的部分。
```
struct leaf_node_t{
    off_t parent_;    // 父节点的偏移
    off_t next_;     // 下一个节点的偏移
    off_t prev_;     // 上一个节点的偏移
    int children_num_;    // 现有的孩子数
    record_t children_[TREE_ORDER];    // 孩子们
};
```
容易看出，leaf_node_t和inner_node_t之间唯一的区别在于孩子的类型，inner_node_t的孩子是index_t类型的，而leaf_node_t的孩子是record_t类型的。
- 内部分支 index_t
<br>即内部节点的孩子分支。
```
struct index_t{
    m_key_t key;
    off_t child;
};
```

- 叶子分支 record_t
<br>即叶子节点的孩子分支，也就是B+树中实际用来储存数据的部分。当外界给出一个键search_key，B+树从最顶端的root开始，一路根据search_key与index_t的key的大小比较，找到符合的叶子节点，然后从record_t中取出search_key对应的value。
```
struct record_t{
    m_key_t key;
    value_t value;
};
```
- B+树元信息 MetaData
<br>由于B+树以文件的形式储存在硬盘里，要对其中的某个节点进行操作，必须要具有该节点对应的偏移值。这些偏移值可以通过该节点的母节点得到。而最高层的根节点的偏移值则需要被储存在文件开头以便直接访问。这些储存在文件开头的数据即B+树的元数据。
```
struct MetaData{
    int max_children_;       // the order of the tree
    int key_size_;           // size of key
    int value_size_;         // size of value
    int height_;             // the height of the tree
    int inner_node_num_;     // numbers of inner nodes in the tree
    int leaf_node_num_;      // numbers of leaf nodes in the tree
    off_t root_offset_;        // where is root stored
    off_t leave_offset_;       // where is the first leave stored
    off_t slot_;               // the newest available place
}；
```
- B+树类型 BPlusTree
<br>此部分内容在下一节阐述。

#### 函数与方法
B+树对外提供以下方法：
- 查找
- 插入
- 删除

B+树的内部方法多且复杂，不光写起来让人崩溃，解释起来更甚。在时间及其紧迫的考试周，为了不挂掉ADS，我不得不选择略过这部分内容，请助教谅解。
接下来介绍B+树的public method：
- 查找
```
// 函数原型
value_t BPlusTree::Search(m_key_t key);
/* 从B+树中查找一个键为key的键值对，返回它的值
    若找不到，则返回-1.
    由于节点在文件中的偏移都是正数，因此返回-1可以说明错误。
    原理如书上所示，而实现涉及到较多内部方法，在此略过 */
```
- 插入
```
// 函数原型
void BPlusTree::Insert(m_key_t key, value_t value);
/* 向B+树中插入一个(key, value)的键值对。
    原理如书上所示，而实现涉及到较多内部方法，在此略过 */
```
- 删除
```
// 函数原型
void BPlusTree::Delete(m_key_t key);
/* 从B+树中删除键为key的记录。
    由于B+树的删除比插入还复杂，因此在本程序中选用了懒惰删除的方法。
    即先找到键为key的节点，将其值设为-1，即完成删除。
    经过这种操作，以后再搜索key，虽然搜索到了该节点，但返回值为-1，
    仍然代表着没有找到。
    而插入操作需要因此做出调整。当需要插入(key, value)的键值对时，
    先查找键为key的节点。若找到了，其值为-1，
    则将其值设为value，即完成插入。
    若没有找到，再进行正常的插入操作。*/
```

### 数据库操作的实现
#### 创建表
以表名为名，在./.index中创建一个新的路径。新建一个B+树，内容为空。此B+树用于存储新表的主键索引。根据和列的序号为该B+树对应的文件取一个独特的文件名，将此B+树写入刚才新建的文件夹中。这意味着此索引属于这张表。

创建工作通过new一个BPlusTree的对象完成，存盘工作通过BPlusTree的Write方法实现。
#### 创建索引
由于每张表在创建时自动生成了主键的索引，因此在对某张表手动添加索引时，该表所对应的目录已经存在了。新建一个B+树，将该表中已有的记录的键值对插入新建的B+树。根据列的序号为该B+树对应的文件取一个独特的文件名，将此B+树写入该表对应的文件夹中。这意味着此索引属于这张表。

创建工作通过new一个BPlusTree的对象完成，存盘工作通过BPlusTree的Write方法实现。
#### 删除表
一个表上可能含有多个索引。但根据设计，同属于一个表的索引都储存在同一个文件夹中。所以进入该表对应的文件夹，将其中的所有文件删除，再删除这个文件夹，即可完成任务。

删除表的操作不通过BPlusTree内建的方法实现，而通过C++与操作系统的交互实现。
#### 删除索引
当指定要删除某张表上某列的索引时，进入./.index目录下该表对应的目录，找出该列对应的索引文件，删除即可。

删除索引的操作不通过BPlusTree内建的方法实现，而通过C++与操作系统的交互实现。
#### 插入记录
因为每张表上可能存在多个索引，而本模块在同一时刻最多只能加载一个索引文件，因此当向某张表插入记录时，需要进入该表对应的目录，轮流加载目录下的每一个索引文件，向其中插入新的记录。

加载工作通过BPlusTree的Read方法实现，插入记录通过BPlusTree的Insert方法实现，存盘工作通过BPlusTree的Write方法实现。
#### 删除记录
因为每张表上可能存在多个索引，而本模块在同一时刻最多只能加载一个索引文件，因此当从某张表删除记录时，需要进入该表对应的目录，轮流加载目录下的每一个索引文件，从其中删除记录。

加载工作通过BPlusTree的Read方法实现，删除记录通过BPlusTree的Delete方法实现，存盘工作通过BPlusTree的Write方法实现。
#### 查询记录
由于本模块在同一时刻最多只能加载一个索引文件，而需要查询的记录很可能不在模块当前加载的索引上，因此需要先判断需要的索引与加载的索引是否相同，若不同需要卸载当前索引并加载所需索引。

然后通过BPlusTree的Search方法查询到所需的值并返回。

加载工作通过BPlusTree的Read方法实现，查询记录通过BPlusTree的Search方法实现。
## 对外接口
### 示例表格：
```
CREATE TABLE person (
    ID varchar(10),
    Name varchar(20),
    Gender varchar(1),
);
```

表格示例如下图所示。其中OFFSET一列指该记录在文件中的偏移值，在真实表格中并不存在。假设每条记录长度为100。

|id|name|gender|OFFSET
|:-:|:-:|:-:|:-:|
|01|a|M|0|
|02|b|M|100|
|03|c|F|200|
|04|d|F|300|
|05|e|M|400|
|06|f|F|500|


### 输出接口
本模块对外提供以下接口：
- 查询名为名为table_name的表中第col_num列是否存在索引。col_num从零开始计数。存在返回true，不存在返回false。
```
// 函数原型
bool IndexManager::CheckExist(string table_name, int col_num);
// 示例：查询表person的gender列是否存在索引。
CheckExist("person", 2);    // return false
```

- 为名为table_name的表中第col_num列创建索引。
```
// 函数原型
void IndexManager::Create(string table_name, int col_num);
// 示例：为person表中的name列创建索引。
Create("person", 1);
```
- 当向名为table_name的表中插入一条记录时，调用此命令以向该表涉及到的所有索引插入新的索引记录。
```
// 函数原型
void IndexManager::Insert(string table_name);
// 示例：向表person中插入(07, g, M)后，调用
Insert(person);        // 自动为id列和name列的索引插入新的索引记录
```
- 删除名为table_name的表的第col_num列的索引文件。
```
// 函数原型
void IndexManager::DeleteIndex(string table_name, int col_num);
// 示例：删除表person中name列的索引
DeleteIndex("person", 1);
```
- 当需要查询名为tablename的表中第colnum列中某条键为key的记录的地址时，调用此条命令。若查找到则返回对应值，否则返回-1。
```
// 函数原型
int IndexManager::Search(string table_name, int col_num, char* key);
// 示例：对于SELECT * FROM person WHERE id = 04; 
    这条命令，需要获取person表中id为04的记录的储存地址偏移。调用
Search("person", 0, "04");        // return 300
// 当根据输入结果无法找到对应记录时，返回-1
Search("person", 0, "123");        // return -1
Search("person", 1, "123");        // return -1
Search("nosrep", 0, "04");        // return -1
```
- 当删除名为tablename的表中的某些记录时，调用此条命令以从该表涉及到的所有索引中删除相关索引记录。
```
// 函数原型
int IndexManager::DeleteKey(string table_name);
// 示例：DELETE FROM person WHERE id = 06;
    对于这条命令，调用
DeleteKey("person");    // 自动删除表person中id列的索引里id=06的索引记录
```

### 输入接口
本模块需要如下输入接口：
- 
```
// 函数原型
vector<vector<string> > QueryTableContent(string table_name);
// 示例：获取表person的内容
QueryTableContent("person");
/* return
{
    {"01", "a", "M", "0"},
    {"02", "b", "M", "100"},
    ...
    {"06", "f", "F", "500"},
}
其中每一行最后一个元素为该记录的OFFSET */
```

- 
```
// 函数原型
vector<string> QueryLatestRecord();
// 示例：刚刚执行了语句 INSERT INTO person value (07, "g", "M"); 此时调用
QueryLatestRecord();
/* return {"07", "g", "M", "600"} 其中最后一个元素600是该记录的偏移值
```
- 
```
// 函数原型
vector<vector<string> > QueryDeletedContent();
// 示例：刚刚执行了语句 DELETE FROM person; 此时调用
QueryDeletedContent();
/* return
{
    {"01", "a", "M"},
    {"02", "b", "M"},
    ...
    {"06", "f", "F"},
} */
```

## 内部接口
- 将输入的表名table_name和列号col_num转化成table_name表col_num列的索引文件的文件路径。同时，如果表的路径不存在，则建立该路径。
该函数的存在是为了方便外部调用，使外部只需要知道表名和列号就能指定某个索引。
```
// 函数原型
string GenerateIndexDirectory(string table_name, int col_num);
// 示例
GenerateIndexDirectory(person, 1);
// return "./.index/person/1", 若./.index/person/目录不存在，则创建该目录
```
- 检查某个以directory为文件路径的索引是否存在。存在返回true，不存在返回false。
本函数与对外借口中的CheckExist()类似，区别在于对外接口中CheckExist()以表名和列号为参数，而此处则直接以索引文件的路径为参数，较之对外的接口更加方便。
```
// 函数原型
bool IndexManager::CheckExist(string directory);
// 示例
CheckExist("./.index/person/1");    // 查询表person的第1列是否有索引
```
- 载入位于某个路径的索引文件。
```
// 函数原型
void IndexManager::LoadIndex(string directory);
/* 示例
    当需要向表house（主键在第0列）中添加数据时，
    由于当前载入的时表person的id列的索引，所
    以需要卸载当前索引，加载表house第0列的索引。
    这一过程通过调用以下函数来完成 */
LoadIndex("./.index/house/0");
```
- 获取某个表的所有索引文件，以供插入与删除记录时轮流加载索引，进行更新。
```
// 函数原型
vector<string> IndexColumnTable(string table_name);
// 示例
    假设表perosn的第0列和第1列都有索引。此时调用
IndexColumnTable(person);    // return {"0", "1"}
```