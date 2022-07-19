#pragma once
#include "event_loop.h"
#include "buffer.h"
class tcp_conn
{
public:
    
    tcp_conn(int connfd,event_loop* loop);

    ~tcp_conn() ;

    void do_read();

    void do_write();

    //int send_message(const char *data,int msglen,int msgid);

    void clean_conn();


private:
    int m_connfd;

    event_loop *m_loop;

    read_buffer r_buf;

    write_buferr w_buf;
};



//======================http_html=======================
class http_html
{
public:
    //接受一个解析完的字符串地址
    http_html(const char *data,int len);

    ~http_html();

    void send_head(int num, const char* st,const char* type,int w_len);

    void send_file();

    void send_dir();

    const char* sent_data();

    int send_len();

    char* get_file_type(const char *name);
private:

    char file[256];

    char data[m8M];

    int len=0;
};