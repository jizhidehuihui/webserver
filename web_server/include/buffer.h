#pragma once
#include "io_buf.h"
#include "bufpool.h"
class buffer
{
public:
    buffer();
    ~buffer();
    //得到当前的buf还有多少有效数据
    int length();

    //已经处理了多少数据
    void pop(int len);

    //将当前的数据清空
    void clear();

protected:
    io_buf* m_buf;
};

class read_buffer:public buffer
{
public:
    //从fd中读取数据到read_buffer中
    int read_fd(int fd);

    //获取当前数据的接口
    const char* read_data();

    //重置缓冲区
    void adjust();
};

class write_buferr:public buffer
{
public:
    //将数据写入系统缓冲区fd中
    int write_fd(int fd);

    //将数据写入到write_buffer中
    int wtite_data(const char *data,int data_len);
};
