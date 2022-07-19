#pragma once
#include <arpa/inet.h>
#include "event_loop.h"
class server
{
public:
    server(event_loop *loop,const char *ip,uint16_t port);
    ~server();

    //提供创建连接的服务
    void do_accept();

    //提供创建和客户端通信的read回调函数
    void add_cfd_read_callback(io_callback read_cb,void* args);
    //提供创建和客户端通信的read回调函数
    void add_cfd_write_callback(io_callback write_cb,void *arg);

private:
    int m_sockfd;//监听的套接字
    struct sockaddr_in m_conaddr;//客户端连接地址
    socklen_t m_addrlen;//客户端连接地址长度

    //epoll 多路复用机制
    event_loop *m_loop;

    //用于通信的回调函数
};

