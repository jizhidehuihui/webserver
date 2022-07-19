#include <iostream>
#include "buffer.h"
#include <sys/ioctl.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

//================read_buff=======================
buffer::buffer()
{
    m_buf=NULL;
}

buffer::~buffer()
{
    this->clear();
}

 //得到当前的buf还有多少有效数据
int buffer::length()
{
    if(m_buf!=NULL)
    {
        return m_buf->length;
    }
    else
    {
        return 0;
    }
}


//已经处理了多少数据
void buffer::pop(int len)
{
    if(m_buf==NULL&&len<m_buf->length)
    {
        fprintf(stderr,"buffer pop error\n");
        exit(1);
    }
    m_buf->pop(len);

    if(m_buf->length==0)
    {
         //当前的_buf已经全部用完了
        this->clear();
    }
}

//将当前的数据清空
void buffer::clear()
{
    if(m_buf!=NULL)
    {
         //将_buf 放回buf_pool中
        bufpool::get_bufpool()->revert(m_buf);
        m_buf=NULL;
    };

}

//================read_buff=======================
//从fd中读取数据到read_buffer中
int read_buffer::read_fd(int fd)
{
    int need_read_size;
    //一次性将io中的缓存数据全部都出来
    //需要给fd设置一个属性
    //传出一个参数,目前缓冲中一共有多少数据是可读
    if(ioctl(fd,FIONREAD,&need_read_size)==-1)
    {
        fprintf(stderr,"ioctl FIONREAD\n");
        return -1;
    }

    if(m_buf==NULL)
    {
        m_buf=bufpool::get_bufpool()->get_io_buf(need_read_size);
            if (m_buf == NULL) {
            fprintf(stderr, "no buf for get!\n");
            return -1;
        }
    }
    else{
        assert(m_buf->head == 0);
        if(m_buf->capacity-m_buf->length<need_read_size)
        {
            //内存不够用
            io_buf* new_buf=bufpool::get_bufpool()->get_io_buf(need_read_size+m_buf->length);
            if (new_buf == NULL) {
            fprintf(stderr, "no buf for alloc\n");
            return -1;
            }

            //将之前的_buf数据拷贝到新的buf中
            new_buf->copy(m_buf);
            //将之前的_buf 放回内存池中
            bufpool::get_bufpool()->revert(m_buf);
            //新申请的buf称为当前的io_buf
            m_buf=new_buf;
        }
    }

    int already_read=0;
    //当前的buf是可以容纳  读取数据
    do{
        if(need_read_size==0)
        {
            already_read=read(fd,m_buf->data+m_buf->length,m4K);
        }
        else{
            already_read=read(fd,m_buf->data+m_buf->length,need_read_size);
        }
    }while(already_read==-1&&errno==EINTR);

    if(already_read>0)
    {
        if(need_read_size!=0)
        {
            assert(already_read == need_read_size);
        }
        m_buf->length+=already_read;
    }

    return already_read;

}

//获取当前数据的接口
const char* read_buffer::read_data()
{
    if(m_buf!=NULL)
    {
        return m_buf->data+m_buf->head;
    } 
    else
    {
        return NULL;
    }
}

//重置缓冲区
void read_buffer::adjust()
{
    if(m_buf!=NULL)
    m_buf->adjust();
}

//====================write_buf==========================

//将数据写入系统缓冲区fd中
int write_buferr::write_fd(int fd)
{
    assert(m_buf!=NULL&&m_buf->head==0);
    int alread_write=0;
    do{
        alread_write=write(fd,m_buf->data,m_buf->length);
    }while(alread_write==-1&&errno==EINTR);//系统调用中断产生，不是一个错误

    if(alread_write>0)
    {
        m_buf->pop(alread_write);
        m_buf->adjust();
    }

 //如果fd是非阻塞的,会报already_write==-1 errno==EAGAIN
    if(alread_write==-1&&errno==EAGAIN)
    {
        alread_write=0;//表示非阻塞导致的-1 不是一个错误,表示是正确的只是写0个字节
    }
    return alread_write;

}

//将数据写入到write_buffer中
int write_buferr::wtite_data(const char *data,int data_len)
{
    if(m_buf==NULL)
    {
        m_buf=bufpool::get_bufpool()->get_io_buf(data_len);
            if (m_buf == NULL) {
            fprintf(stderr, "no buf for get!\n");
            return -1;
        }
    }
    else{
         assert(m_buf->head == 0);
        if(m_buf->capacity-m_buf->length<data_len)
        {
            //内存不够用
            io_buf* new_buf=bufpool::get_bufpool()->get_io_buf(data_len+m_buf->length);
            if (new_buf == NULL) {
            fprintf(stderr, "no buf for alloc\n");
            return -1;
            }

            //将之前的_buf数据拷贝到新的buf中
            new_buf->copy(m_buf);
            //将之前的_buf 放回内存池中
            bufpool::get_bufpool()->revert(m_buf);
            //新申请的buf成为当前的io_buf
            m_buf=new_buf;
        }
    }
    memcpy(m_buf->data+m_buf->length,data,data_len);
    m_buf->length+=data_len;
    return 0;
};
