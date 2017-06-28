# IndexManager 设计说明

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

