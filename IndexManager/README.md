# IndexManager 设计说明 V1.0

## 调用方法
```
IndexManager idx;
```
### 查询某索引是否存在
#### 调用idx.CheckExist()
```
// bool IndexManager::CheckExist(string directory);
// directory是需要查询的索引的名称
idx.CheckExist(表名_列名);
```

### 建立新的表时 or 通过命令建立索引
#### 调用idx.Create()
```
// void IndexManager::Create(string directory);
// directory 是该新建索引的名称
idx.Create(表名_列名);
```
#### 如何获取需要插入的键值对？
不知道。

### 查找键对应的值
#### 调用idx.Search()
```
// int IndexManager::Search(string directory, char* key);
// directory是需要使用的索引的名称，key是键
idx.Search(表名_列名, key);
```
#### 返回值
- 找到则返回键值，找不到则返回-1
- 找不到既有可能是没有建立过该索引，也可能是索引中不存在该条目。

### 每次插入涉及到索引的记录时
#### 调用idx.Insert()
```
// void IndexManager::Insert(string directory, char* key, int value);
// directory是需要使用的索引文件，key是键，value是键值。
idx.Insert(表名_列名, key, value);
```

### 删除索引
#### 调用idx.Delete()
```
// void IndexManager::Delete(string directory);
// directory是需要删除的索引的名称。
idx.Delete(表名_列名);
```


## 使用
只需在main.cpp的开头添加
```
#include<IndexManager.h>
```
即可使用IndexManager.

## 初始化
```
IndexManager idx(string index_name);
```
即可在当前目录下创建一个以**index_name**命名的索引文件，或者从当前目录下读取一个以**index_name**命名的已经存在的索引文件。

**NOTE:** 建议以 **表名_列名** 命名索引文件。如对于表student中的id列，其对应的索引文件应被命名为student_id.index.

## 插入
```
idx.Insert(char* key, int value);
```
即可向索引文件中插入键为key，键值为value的一条记录。其中key为char*，要求长度不得超过32。value为int类型。

## 搜索
```
int result = idx.Search(string index_name, char* key);
```
即可从索引文件中查询一条键为key的记录，结果将以int的形式返回给result。当搜索的key不存在时，结果返回-1.
