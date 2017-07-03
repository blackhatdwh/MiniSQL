### 文件列表
**B+树**
- b_plus_tree.h
- b_plus_tree.cc

**IndexManager主体**
- IndexManager.h
- IndexManager.cc

**与下层模块的接口**
- Interaction.h
- Interaction.cc

**测试代码**
- main.cc

**可执行文件**
- a.out

### 编译环境

- 操作系统 deepin 15.4
- 编译器   gcc version 6.3.0
- 运行
```
rm -rf ./.index/
g++ main.cc IndexManager.cc b_plus_tree.cc Interaction.cc
```
即可编译得到a.out。运行a.out即可进行测试。

### 说明
本模块实现了如下功能：
- 建立索引
- 插入记录
- 查找记录
- 删除记录
- 删除索引
- 提供接口

且均通过测试。测试代码在main.cc中给出。

但由于时间紧张且协调配合失当，最终未能完成整合。

本模块运行中产生的索引文件都储存在./.index/下，在进行测试前最好先将目录删除，以避免之前的测试的影响。