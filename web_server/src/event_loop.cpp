#include <iostream>
#include "event_loop.h"
//构造 用于创建epoll
event_loop::event_loop()
{
   
    m_epft=epoll_create1(0);
    if(m_epft==-1)
    {
        fprintf(stderr,"epoll_create error\n");
        exit(1);
    }
    std::cout<<"=========event_loop\n";
}
event_loop::~event_loop()
{

}

void event_loop::evebt_process()
{
    map<int,io_event>::iterator ev_it;
    while(true)
    {
        std::cout<<"=========event_process\n";
        int nfds=epoll_wait(m_epft,m_fired_evs,MAXEVENTS,-1);
        for(int i=0;i<nfds;i++)
        {
            ev_it=m_io_evs.find(m_fired_evs[i].data.fd);
            io_event* ev=&(ev_it->second);
            if(m_fired_evs[i].events&EPOLLIN)
            {
                //读事件， 调用读回调函数
                void *args=ev->rcb_args;
                ev->read_callback(this,m_fired_evs[i].data.fd,args);
            }
            else if(m_fired_evs[i].events&EPOLLOUT){
                 //写事件, 调用写回调函数
                 void *args=ev->wcb_args;
                 ev->write_callback(this,m_fired_evs[i].data.fd,args);
            }
            else if (m_fired_evs[i].events & (EPOLLHUP|EPOLLERR)) {
            //水平触发未处理， 可能会出现HUP事件，需要正常处理读写， 如果当前时间events 既没有写，也没有读 将events从epoll中删除
            if (ev->read_callback != NULL) {
                //读事件， 调用读回调函数
                void *args = ev->rcb_args;
                //调用读业务
                ev->read_callback(this, m_fired_evs[i].data.fd, args);
            }
            else if (ev->write_callback != NULL) {
                //写事件, 调用写回调函数
                void *args = ev->wcb_args;
                ev->write_callback(this, m_fired_evs[i].data.fd, args);
            }
            else  {
                //删除
                fprintf(stderr, "fd %d get error, delete from epoll", m_fired_evs[i].data.fd);
                this->del_io_event(m_fired_evs[i].data.fd);
            }
            }
        }
    }
}


//添加一个io事件到event_loop中
void event_loop::add_io_event(int fd, io_callback *proc, int mask, void *args)
{
    int final_mask;
    int op;

     //找到当前的fd是否已经是已有事件
    auto it=m_io_evs.find(fd);
    if(it==m_io_evs.end())
    {
         // 如果没有事件， ADD方式添加到epoll中
        op=EPOLL_CTL_ADD;
        final_mask=mask;
    }
    else{
         // 如果事件存在, MOD方式添加到epoll中
        op=EPOLL_CTL_MOD;
        final_mask=it->second.mask|mask;
    }

    if(mask&EPOLLIN)
    {
        //读事件
        m_io_evs[fd].read_callback=proc;
        m_io_evs[fd].rcb_args=args;
    }
    else if(mask&EPOLLOUT)
    {
        //写事件
        m_io_evs[fd].write_callback=proc;
        m_io_evs[fd].wcb_args=args;
    }

    m_io_evs[fd].mask=final_mask;


    //将事件和epoll事件绑定
    struct epoll_event event;
    event.events=final_mask;
    event.data.fd=fd;
    if(epoll_ctl(m_epft,op,fd,&event)==-1)
    {
        fprintf(stderr, "epoll ctl %d error\n", fd);
        return;
    }

    //添加到监听的集合中
    m_listen_fds.insert(fd);
}
//删除一个io事件 从event_loop中
void event_loop::del_io_event(int fd)
{
    auto it=m_io_evs.find(fd);
    if(it==m_io_evs.end())
    {
        return;
    }

    m_io_evs.erase(fd);

    m_listen_fds.erase(fd);

    epoll_ctl(m_epft,EPOLL_CTL_DEL,fd,NULL);

}

//删除一个io事件的某个触发条件(EPOLLIN/EPOLLOUT)
void event_loop::del_io_event(int fd, int mask)
{
    auto it=m_io_evs.find(fd);
    if(it==m_io_evs.end())
    {
        return ;
    }

    int t_mask=m_io_evs[fd].mask;
    t_mask=t_mask&(~mask);

    if(t_mask==0)
    {
         //通过删除条件已经没有触发条件
        del_io_event(fd);
    }
    else{
        struct epoll_event event;
        event.data.fd=fd;
        event.events=t_mask;
        epoll_ctl(m_epft,EPOLL_CTL_MOD,fd,&event);
    }
}


