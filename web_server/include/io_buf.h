#pragma once

class io_buf
{
public:
//构造函数  通过size大小创建buf
    io_buf(int size);
    ~io_buf(){};

    //清空数据
    void clear();

    //处理长度为len的数据
    void pop(int len);

    //将已经处理的数据清空，将未处理的数据前移
    void adjust();

    //将其他io_buf拷贝到当前io_buf中
    void copy(io_buf *);

    //内存的大小
    int capacity;

    //当前buf的有效数据
    int length;

    //当前buf的头部索引
    int head;

    //当前buf的内存首部地址
    char *data;

    //存在多个io_buf 采用链表的结构
    io_buf *next;
};