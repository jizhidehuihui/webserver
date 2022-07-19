#include <iostream>
#include "../include/http_html.h"
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
void conn_rd_callback(event_loop *loop,int fd,void *args);
void conn_wr_callback(event_loop *loop,int fd,void *args);

tcp_conn::tcp_conn(int connfd,event_loop* loop)
{
    m_connfd=connfd;
    m_loop=loop;

    int flag=fcntl(m_connfd,F_SETFL,0);
    fcntl(m_connfd,F_SETFL,O_NONBLOCK);

     //2 设置TCP_NODELAY状态， 禁止读写缓存，降低小包延迟
    int op = 1;
    setsockopt(m_connfd, IPPROTO_TCP, TCP_NODELAY, &op, sizeof(op));//need netinet/in.h netinet/tcp.h

    m_loop->add_io_event(m_connfd,conn_rd_callback,EPOLLIN,this);

}

tcp_conn::~tcp_conn() 
{

}

void tcp_conn::do_read()
{
    int ret=r_buf.read_fd(m_connfd);

    if (ret == -1) {
        fprintf(stderr, "read data from socket\n");
        this->clean_conn();
        return;
    }
    else if (ret == 0) {
        //对端客户端正常关闭
        printf("peer client closed!\n");
        this->clean_conn();
        return;
    }


    bool active_epollout = false;
        if (w_buf.length() == 0) {
        //现在obuf是空的, 之前的数据已经都发送完了，需要再次发送 需要激活epoll的写事件回掉，
        //因为如果obuf不为空， 说明数据还没有完全写到对端，那么久没必要再激活，等写完再激活
        active_epollout = true;
    }

    //解析http请求字符串并穿件http_html对象
    int len=r_buf.length();
    char method[12],path[256];
    sscanf(r_buf.read_data(),"%[^ ] %[^ ] ",method,path);

    printf("%s %s\n",method,path);

    http_html* html=new http_html(path,strlen(path));

    std::cout<<html->sent_data()<<std::endl;
    std::cout<<html->send_len()<<std::endl;
    //清空read_buffer的数据
    r_buf.pop(len);
    r_buf.adjust();

    //将处理完的数据放入到write_buffer中
    w_buf.wtite_data(html->sent_data(),html->send_len());


    //注册写事件

        if (active_epollout == true) 
    m_loop->add_io_event(m_connfd,conn_wr_callback,EPOLLOUT,this);

        
 
}

void tcp_conn::do_write()
{
    while(w_buf.length())
    {
        int write_num=w_buf.write_fd(m_connfd);
       
        if (write_num == -1) {
            fprintf(stderr, "tcp_conn write connfd error\n");
            this->clean_conn();
            return;
        }
        else if(write_num==0)
        {
            break;
        }
    }
     std::cout<<w_buf.length();
    if(w_buf.length()==0)
    {
        m_loop->del_io_event(m_connfd,EPOLLOUT);
    }
    return ;
}


void tcp_conn::clean_conn()
{
    m_loop->del_io_event(m_connfd);

    r_buf.clear();
    w_buf.clear();

    close(m_connfd);
}


void conn_rd_callback(event_loop *loop,int fd,void *args)
{
    tcp_conn *conn=(tcp_conn*)args;
    conn->do_read();
}
void conn_wr_callback(event_loop *loop,int fd,void *args)
{
    tcp_conn *conn=(tcp_conn*)args;
    conn->do_write();
    
}




//======================http_html=======================

http_html::http_html(const char *path,int len)
{
    bzero(data,m8M);
    memcpy(file,"1.jpg",strlen("1.jpg"));
    send_head(200,"ok","1.jpg",0);
    send_file();
}

void http_html::send_head(int num,char* st,char* type,int w_len)
{
    len+=sprintf(data,"http/1.1 %d %s\r\n",num,st);
    len+=sprintf(data+len,"Content-Type:%s\r\n",get_file_type(type));
    len+=sprintf(data+len,"Content-Lenght:%d\r\n",w_len);
    len+=sprintf(data+len,"\r\n",2,0);
}

void http_html::send_file()
{

    int fd=open(file,O_RDONLY);
    int r_len=0;
    do{
        r_len=read(fd,data+len,40960);
        len+=r_len;
    }while(r_len>0);
    
    close(fd);
    
}

void http_html::send_dir()
{

}

char* http_html::get_file_type(const char *name)
{
    const char *dot;
    dot=strrchr(name,'.');
    if(dot==NULL)
    return "text/plain; charset=utf-8";
    if(strcmp(dot,".html")==0||strcmp(dot,"htm")==0)
    return "text/html; charset=utf-8";
    if(strcmp(dot,".jpg")==0||strcmp(dot,".jpeg")==0)
    return "image/jpeg";
    if(strcmp(dot,".gif")==0)
    return "imag/gif";
    if(strcmp(dot,".png")==0)
    return "imag/png";
    if(strcmp(dot,".css")==0)
    return "text/css";
    if(strcmp(dot,".au")==0)
    return "audio/basic"; 
    if(strcmp(dot,".wav")==0)
    return "audio/wab"; 
    if(strcmp(dot,".avi")==0)
    return "video/x-msvideo"; 
    if(strcmp(dot,".mov")==0)
    return "video/quicktime";
    if(strcmp(dot,".mpeg")==0)
    return "voide.mpeg";
    if(strcmp(dot,".vrml")==0)
    return "model/vrml"; 
    if(strcmp(dot,".midi")==0)
    return "audio/midi"; 
    if(strcmp(dot,".mp3")==0)
    return "audio/mpeg";
    if(strcmp(dot,".ogg")==0)
    return "application/ogg";
    
    return "text/html; charset=utf-8";
}


const char* http_html::sent_data()
{
    return data;
}

int http_html::send_len()
{
    return len;
}