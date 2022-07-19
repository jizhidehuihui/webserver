#pragma once
#include <pthread.h>
#include "io_buf.h"
#include <map>
#define MEM_MAX (5U*1024*1024) //内存池总大小上限

using std::map;


//内存池提前申请的大小
enum MEM_CAP
{
    m4K = 4096,
    m16K = 16384,
    m64K = 65536,
    m256K = 262144,
    m1M = 1048576,
    m4M = 4194304,
    m8M = 8388608,
    m64M = 67108864
};

class bufpool
{
public:
    ~bufpool();

    //初始化单例对象
    //获取单例对象的外部接口
    static bufpool* get_bufpool()
    {
        return m_bufpool;
    }

    //申请一块内存 
    io_buf *get_io_buf(int size);
    io_buf *get_io_buf();

    //将内存放回内存池
    void revert(io_buf *buffer);


    void make_io_buf_list(int cap,int num);

private:
    //创造单例 所有构造函数私有化
    bufpool();
    bufpool(const bufpool &);
    const bufpool&  operator=(const bufpool &);

    //唯一单例对象
    static bufpool *m_bufpool;

    //存放iobuf的所有句柄的map
    map<int,io_buf*> m_pool;

    //内存池已使用的大小
    uint64_t m_total_mem=0;

    //保护pool的锁
    static pthread_mutex_t m_mutex;
};

