#include <iostream>
#include <signal.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include "buffer.h"
#include "server.h"
#include "../include/http_html.h"

void cfd_write_callback(event_loop *loop,int fd,void* args);
void cfd_read_callback(event_loop *loop,int fd,void *args);

//临时的收发消息结构
struct message
{
    char data[m4M];
    int len;
};
struct message msg;



void accept_callback(event_loop *loop,int fd,void *args)
{
    std::cout<<"=========accept_callback\n";
    server *ser=(server*)args;
    ser->do_accept();
}


server::server(event_loop *loop,const char *ip,uint16_t port)
{
    //忽略一些信号
    if (signal(SIGHUP, SIG_IGN) == SIG_ERR) 
    {
        fprintf(stderr, "signal ignore SIGHUB\n");
    }

    if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)  {
        fprintf(stderr, "signal ignore SIGHUB\n");
    }

    //创建服务器监听套接字
    m_sockfd =socket(AF_INET,SOCK_STREAM|SOCK_CLOEXEC,0);
    if(m_sockfd==-1)
    {
        fprintf(stderr,"server socket error\n");
        exit(1);
    }

    //初始化服务器地址信息
    struct sockaddr_in server_addr;
    bzero(&server_addr,sizeof(server_addr));
    server_addr.sin_family=AF_INET;
    inet_aton(ip,&server_addr.sin_addr);
    server_addr.sin_port=htons(port);

    //设置端口复用
    int op=1;
    if(setsockopt(m_sockfd,SOL_SOCKET,SO_REUSEADDR,&op,sizeof(op))<0)
    {
        fprintf(stderr,"sert socketopt error\n");
    }

    //绑定监听
    if(bind(m_sockfd,(const struct sockaddr*)&server_addr,sizeof(server_addr))<0)
    {
        fprintf(stderr,"server bind error\n");
        exit(1);
    }
    if(listen(m_sockfd,128)<0)
    {
        fprintf(stderr,"server listen error\n");
        exit(1);
    }

    m_loop=loop;
    m_loop->add_io_event(m_sockfd,accept_callback,EPOLLIN,this);

}

server::~server()
{

}

void server::do_accept()
{
    int connfd;
    while(true)
    {
        connfd=accept(m_sockfd,(struct sockaddr*)&m_conaddr,&m_addrlen);
        if(connfd==-1)
        {
            if(errno==EINTR)
            {
                //中断错误
                fprintf(stderr, "accept errno = EINTR\n");
                continue;
            }
            else if (errno == EAGAIN) {
                fprintf(stderr, "accept errno = EAGAIN\n");
                break;
            }
            else if(errno==EMFILE)
            {
                 //建立链接过多， 资源不够
                fprintf(stderr, "accept errno = EMFILE\n");
                continue;
            }
            else{
                fprintf(stderr, "accept error");
                exit(1);
            }
        }
        else
        {
           // m_loop->add_io_event(connfd,cfd_read_callback,EPOLLIN,NULL);
           // return ;
            //accept succ!
            printf("accept succ! fd is%d\n",connfd);

            //创建一个新的tcp_conn连接对象
            tcp_conn *conn = new tcp_conn(connfd, m_loop);
            if (conn == NULL) {
                fprintf(stderr, "new tcp_conn error\n");
                exit(1);
            }

            printf("get new connection succ!\n");
            break;
        }
    }
}





#if 0
void cfd_read_callback(event_loop *loop,int fd,void *args)
{
    struct message *msg=new message();
    read_buffer rbuf;
    int ret=rbuf.read_fd(fd);
    if (ret == -1) 
    {

    fprintf(stderr, "ibuf read_data error\n");
    //当前的读事件删除
    loop->del_io_event(fd);

    //关闭对端fd
    close(fd);
    return ;
    }

    if (ret == 0) 
    {
    //对方正常关闭
    //当前的读事件删除
    loop->del_io_event(fd);

    //关闭对端fd
    close(fd);
    return ;
    }

    //将读到的数据拷贝到msg中
    msg->len = rbuf.length();
    bzero(msg->data, msg->len);
    memcpy(msg->data, rbuf.read_data(), msg->len);

    
    char method[12],path[256],protocol[12];
    sscanf(rbuf.read_data(),"%[^ ] %[^ ] ",method,path);



    rbuf.pop(msg->len);
    rbuf.adjust();



    printf("%s\n", msg->data);
    printf("method:%s path:%s protocol:%s\n",method,path,protocol);

    loop->del_io_event(fd,EPOLLIN);
    loop->add_io_event(fd,cfd_write_callback,EPOLLOUT,NULL);

void cfd_write_callback(event_loop *loop,int fd,void* args)
{
    /*
    write_buferr obuf;

    //将msg ---> obuf
    obuf.wtite_data(msg->data, msg->len);
    while (obuf.length()) {
        int write_num = obuf.write_fd(fd);
        if (write_num == -1) {
            fprintf(stderr, "write connfd error\n");
            return;
        }
        else if (write_num == 0) {
            //当前不可写
            break;
        }
    }
    */

    //删除写事件， 添加读事件
    loop->del_io_event(fd, EPOLLOUT);
    loop->add_io_event(fd, cfd_read_callback, EPOLLIN, NULL);
}


void cfd_read_callback(event_loop *loop,int fd,void *args)
{
    struct message *msg=new message();
    read_buffer rbuf;
    int ret=rbuf.read_fd(fd);
    if (ret == -1) 
    {

    fprintf(stderr, "ibuf read_data error\n");
    //当前的读事件删除
    loop->del_io_event(fd);

    //关闭对端fd
    close(fd);
    return ;
    }

    if (ret == 0) 
    {
    //对方正常关闭
    //当前的读事件删除
    loop->del_io_event(fd);

    //关闭对端fd
    close(fd);
    return ;
    }

    //将读到的数据拷贝到msg中
    msg->len = rbuf.length();
    bzero(msg->data, msg->len);
    memcpy(msg->data, rbuf.read_data(), msg->len);

    
    char method[12],path[256],protocol[12];
    sscanf(rbuf.read_data(),"%[^ ] %[^ ] ",method,path);



    rbuf.pop(msg->len);
    rbuf.adjust();



    printf("%s\n", msg->data);
    printf("method:%s path:%s protocol:%s\n",method,path,protocol);

    loop->del_io_event(fd,EPOLLIN);
    loop->add_io_event(fd,cfd_write_callback,EPOLLOUT,NULL);


}
}
#endif
