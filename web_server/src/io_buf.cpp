#include "io_buf.h"
#include <stdio.h>
#include <string.h>
//构造函数  通过size大小创建buf
io_buf::io_buf(int size)
{
    capacity=size;
    length=0;
    head=0;
    next=nullptr;
    data=new char[size];
    if(data==nullptr)
    {
        fprintf(stderr,"io_buf new data error\n");
    }
}

//清空数据
void io_buf::clear()
{
    length=head=0;
}

//处理长度为len的数据
void io_buf::pop(int len)
{
    length-=len;
    head+=len;
}

//将已经处理的数据清空，将未处理的数据前移
void io_buf::adjust()
{
    if(head!=0)
    {
        head=0;
        if(length!=0)
        memmove(data,data+head,length);
    }
}

//将其他io_buf拷贝到当前io_buf中
void io_buf::copy(io_buf * buf)
{
    memcpy(data,buf->data+head,buf->length);
    length=buf->length;
    head=0;
}