#include <iostream>
#include "bufpool.h"

//唯一全局bufpool单例对象
bufpool* bufpool::m_bufpool=new bufpool();
pthread_mutex_t bufpool::m_mutex=PTHREAD_MUTEX_INITIALIZER;


//创造单例 所有构造函数私有化
bufpool::bufpool()
{
    make_io_buf_list(m4K, 5000);
    make_io_buf_list(m16K, 1000);
    make_io_buf_list(m64K, 500);
    make_io_buf_list(m256K,200);
    make_io_buf_list(m1M, 50);
    make_io_buf_list(m4M, 20);
    make_io_buf_list(m8M, 10);
    make_io_buf_list(m64M,5);
}

//申请一块内存 
io_buf *bufpool::get_io_buf(int size)
{
        int index;
    // 1 找到N最接近的 一个刻度链表 返回一个io_buf
    if (size <= m4K) {
        index = m4K;
    }
    else if (size <= m16K) {
        index = m16K;
    }
    else if (size <= m64K) {
        index = m64K;
    }
    else if (size <= m256K) {
        index = m256K;
    }
    else if (size <= m1M) {
        index = m1M;
    }
    else if (size <= m4M) {
        index = m4M;
    }
    else if (size <= m8M) {
        index = m8M;
    }
    else if(size <=m64M)
    {
        index = m64M;
    }
    else {
        return NULL;
    }

    pthread_mutex_lock(&m_mutex);
    if(m_pool[index]==NULL)
    {
        if(m_total_mem+index/1024>MEM_MAX)
        {
            fprintf(stderr,"MEM_MAX not enoght\n");
            exit(1);
        }
        io_buf *buffer=new io_buf(index);
        if(buffer==NULL)
        {
            fprintf(stderr,"new io_buf error\n");
            exit(1);
        }
        m_total_mem+=index/1024;
        pthread_mutex_unlock(&m_mutex);
        return buffer;
    }

    io_buf *temp=m_pool[index];
    m_pool[index]=m_pool[index]->next;

    pthread_mutex_unlock(&m_mutex);
    temp->next=NULL;
    return temp;

}
io_buf *bufpool::get_io_buf()
{
    return get_io_buf(m4K);
}

//将内存放回内存池
void bufpool::revert(io_buf *buffer)
{
    int index=buffer->capacity;
    buffer->head=0;
    buffer->length=0;

    pthread_mutex_lock(&m_mutex);

    if(m_pool.find(index)==m_pool.end())
    {
        fprintf(stderr,"m_pool find io_buf error\n");
        exit(1);
    }
    buffer->next=m_pool[index];
    m_pool[index]=buffer;

    pthread_mutex_unlock(&m_mutex);
}

//申请内存辅助函数
void bufpool::make_io_buf_list(int cap,int num)
{
    io_buf* temp=NULL;
    m_pool[cap]=new io_buf(cap);

    if(m_pool[cap]==NULL)
    {
        fprintf(stderr,"m_pool new error\n");
        exit(1);
    }

    temp=m_pool[cap];
    for(int i=1;i<num;i++)
    {
        temp->next=new io_buf(cap);
        if(temp->next==NULL)
        {
            fprintf(stderr,"new io_buf error\n");
            exit(1);
        }
        temp=temp->next;
    }
    m_total_mem+=cap*num/1024;
}


