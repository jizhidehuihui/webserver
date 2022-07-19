#pragma once
#include <map>
#include <set>
#include <sys/epoll.h>
class event_loop;
using std::map;
using std::set;
using io_callback=void(event_loop *loop,int fd,void *args);
#define MAXEVENTS 10

//================封装一次IO触发的事件==================

struct io_event
{
    io_event()
    {
        mask = 0;
        write_callback = NULL;
        read_callback = NULL;
        rcb_args = NULL;
        wcb_args = NULL;
    };

//事件的读写属性
int mask; //EPOLLIN, EPOLLOUT 

//读事件触发所绑定的回调函数
io_callback *read_callback;  

//写事件触发所绑定的回调函数
io_callback *write_callback;

//读事件回调函数的形参
void *rcb_args;

//写事件回调函数的形参
void *wcb_args;
};


//==================封装epoll事件=====================
class event_loop
{
public:
    //构造 用于创建epoll
    event_loop();
    ~event_loop();
    

    //循环监听epoll事件
    void evebt_process();
    //添加一个io事件到event_loop中
    void add_io_event(int fd, io_callback *proc, int mask, void *args);
    //删除一个io事件 从event_loop中
    void del_io_event(int fd);
    
    //删除一个io事件的某个触发条件(EPOLLIN/EPOLLOUT)
    void del_io_event(int fd, int mask);

private:
    int m_epft;

    //存储和fd绑定的读写事件
    map<int,io_event> m_io_evs;

    //存储监听中的事件
    set<int> m_listen_fds;

    struct epoll_event m_fired_evs[MAXEVENTS];
};

