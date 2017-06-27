#IndexManager 设计说明 V1.0

## 使用
只需在main.cpp的开头添加
```
#include<IndexManager.h>
```
即可使用IndexManager.

## 初始化
```
IndexManager idx("文件名");
```
即可在当前目录下创建一个以“文件名“命名的索引文件，或者从当前目录下读取一个以”文件名”命名的已经存在的索引文件。
<br>**NOTE:**建议以表的名称命名索引文件。如对于表student，其对应的索引文件应被命名为student.index.

## 插入
```
idx.Insert(char* key, int value);
```
即可向索引文件中插入键为key，键值为value的一条记录。其中key为char×，要求长度不得超过32。value为int类型。

## 搜索
```
int result = idx.Search(char* key);
```
即可从索引文件中查询一条键为key的记录，结果将以int的形式返回给result。当搜索的key不存在时，结果返回-1.
