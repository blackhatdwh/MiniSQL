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


### 数据库操作的映射
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
- 将输入的表名table_name和列号col_num转化成table_name表col_num列的索引文件的文件地址。
- 检查某个以directory为文件路径的索引是否存在。存在返回true，不存在返回false。
本函数与对外借口中的CheckExist()类似，区别在于对外接口中CheckExist()以表名和列号为参数，而此处则直接以文件名为参数。实际上对外的CheckExist()
```
// 函数原型
bool IndexManager::CheckExist(string directory);
```