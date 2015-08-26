
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_CONNECTION_H_INCLUDED_
#define _NGX_CONNECTION_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>


typedef struct ngx_listening_s  ngx_listening_t;

struct ngx_listening_s {
    ngx_socket_t        fd;                        /* socket文件描述符 */

    struct sockaddr    *sockaddr;                  /* socket地址 */
    socklen_t           socklen;                   /* size of sockaddr */
    size_t              addr_text_max_len;         /*  add_text的最大长度 */
	ngx_str_t           addr_text;                 /* 字符串地址,形如192.168.1.1:80 */

    int                 type;                      /* 套接字类型。types是SOCK_STREAM时，表示是tcp */

    int                 backlog;                   /* TCP实现监听时的backlog队列，它表示允许正在通过三次握手建立tcp连接但还没有任何进程开始处理的连接最大个数 */
	int                 rcvbuf;                    /* 接收缓冲区大小 */
    int                 sndbuf;                    /* 发送缓冲区大小 */
#if (NGX_HAVE_KEEPALIVE_TUNABLE)
    int                 keepidle;
    int                 keepintvl;
    int                 keepcnt;
#endif

    /* handler of accepted connection */
    ngx_connection_handler_pt   handler;           /* 当新的tcp连接成功建立后的处理方法 */
    /* 目前主要用于HTTP或者mail等模块，用于保存当前监听端口对应着的所有主机名 */
    void               *servers;                   /* array of ngx_http_in_addr_t, for example */

    ngx_log_t           log;
    ngx_log_t          *logp;

    size_t              pool_size;                 /* 如果为新的tcp连接创建内存池，则内存池的初始大小应该是pool_size */
    /* should be here because of the AcceptEx() preread */
    size_t              post_accept_buffer_size;
    /* should be here because of the deferred accept */
    ngx_msec_t          post_accept_timeout;       /* accept的超时时间 */

    ngx_listening_t    *previous;                  /* 维护一个链表 执行热代码替换和reconfig时previous指向old_cycle中的相同监听地址的ngx_listening_t对象 */
    ngx_connection_t   *connection;                /* 监听也是一个连接，要分配给监听一个连接资源 */

    ngx_uint_t          worker;                    /* 计数 */

    unsigned            open:1;                    /* 为1表示监听句柄有效，为0表示正常关闭 */
    unsigned            remain : 1;                /* 为1表示不关闭原先打开的监听端口，为0表示关闭曾经打开的监听端口 */
    unsigned            ignore : 1;                /* 标志是否忽略该套接字 */

    unsigned            bound:1;       /* already bound */
    unsigned            inherited:1;   /* inherited from previous process */
    unsigned            nonblocking_accept:1;
    unsigned            listen:1;                  /* 为1表示当前结构体对应的套接字已经监听 */
    unsigned            nonblocking:1;
    unsigned            shared:1;    /* shared between threads or processes */
    unsigned            addr_ntop:1;               /* 为1表示将网络地址转变为字符串形式的地址 */

#if (NGX_HAVE_INET6 && defined IPV6_V6ONLY)
    unsigned            ipv6only:1;
#endif
#if (NGX_HAVE_REUSEPORT)
    unsigned            reuseport : 1;             /* SO_REUSEPORT */
    unsigned            add_reuseport:1;
#endif
    unsigned            keepalive:2;

#if (NGX_HAVE_DEFERRED_ACCEPT)
    unsigned            deferred_accept:1;
    unsigned            delete_deferred:1;
    unsigned            add_deferred:1;
#ifdef SO_ACCEPTFILTER
    char               *accept_filter;
#endif
#endif
#if (NGX_HAVE_SETFIB)
    int                 setfib;
#endif

#if (NGX_HAVE_TCP_FASTOPEN)
    int                 fastopen;                  /* TCP_FASTOPEN */
#endif

};


typedef enum {
     NGX_ERROR_ALERT = 0,
     NGX_ERROR_ERR,
     NGX_ERROR_INFO,
     NGX_ERROR_IGNORE_ECONNRESET,
     NGX_ERROR_IGNORE_EINVAL
} ngx_connection_log_error_e;


typedef enum {
     NGX_TCP_NODELAY_UNSET = 0,
     NGX_TCP_NODELAY_SET,
     NGX_TCP_NODELAY_DISABLED
} ngx_connection_tcp_nodelay_e;


typedef enum {
     NGX_TCP_NOPUSH_UNSET = 0,
     NGX_TCP_NOPUSH_SET,
     NGX_TCP_NOPUSH_DISABLED
} ngx_connection_tcp_nopush_e;


#define NGX_LOWLEVEL_BUFFERED  0x0f
#define NGX_SSL_BUFFERED       0x01
#define NGX_SPDY_BUFFERED      0x02


struct ngx_connection_s {
    void               *data;                /* 维护一个ngx_connection_s的链表 */
    ngx_event_t        *read;                /* 读事件 */
    ngx_event_t        *write;               /* 写事件 */

    ngx_socket_t        fd;                  /* 连接套接字 */

    ngx_recv_pt         recv;                /* 读函数 */
    ngx_send_pt         send;                /* 写函数 */
    ngx_recv_chain_pt   recv_chain;          /* 批量读函数 */
    ngx_send_chain_pt   send_chain;          /* 批量写函数 */

    ngx_listening_t    *listening;           /* 该连接对应的监听 */

    off_t               sent;

    ngx_log_t          *log;

    ngx_pool_t         *pool;

    struct sockaddr    *sockaddr;            /* accept函数返回的套接口的地址 */
    socklen_t           socklen;
    ngx_str_t           addr_text;

    ngx_str_t           proxy_protocol_addr;

#if (NGX_SSL)
    ngx_ssl_connection_t  *ssl;
#endif

    struct sockaddr    *local_sockaddr;      /* accept函数监听的地址 */
    socklen_t           local_socklen;

    ngx_buf_t          *buffer;

    ngx_queue_t         queue;

    ngx_atomic_uint_t   number;

    ngx_uint_t          requests;

    unsigned            buffered:8;

    unsigned            log_error:3;     /* ngx_connection_log_error_e */

    unsigned            unexpected_eof:1;
    unsigned            timedout:1;
    unsigned            error:1;
    unsigned            destroyed:1;

    unsigned            idle : 1;         /* 空闲连接 */
    unsigned            reusable:1;
    unsigned            close:1;

    unsigned            sendfile:1;
    unsigned            sndlowat:1;
    unsigned            tcp_nodelay:2;   /* ngx_connection_tcp_nodelay_e */
    unsigned            tcp_nopush:2;    /* ngx_connection_tcp_nopush_e */

    unsigned            need_last_buf:1;

#if (NGX_HAVE_IOCP)
    unsigned            accept_context_updated:1;
#endif

#if (NGX_HAVE_AIO_SENDFILE)
    unsigned            busy_count:2;
#endif

#if (NGX_THREADS)
    ngx_thread_task_t  *sendfile_task;
#endif
};


#define ngx_set_connection_log(c, l)                                         \
                                                                             \
    c->log->file = l->file;                                                  \
    c->log->next = l->next;                                                  \
    c->log->writer = l->writer;                                              \
    c->log->wdata = l->wdata;                                                \
    if (!(c->log->log_level & NGX_LOG_DEBUG_CONNECTION)) {                   \
        c->log->log_level = l->log_level;                                    \
    }


ngx_listening_t *ngx_create_listening(ngx_conf_t *cf, void *sockaddr,
    socklen_t socklen);
ngx_int_t ngx_clone_listening(ngx_conf_t *cf, ngx_listening_t *ls);
ngx_int_t ngx_set_inherited_sockets(ngx_cycle_t *cycle);
ngx_int_t ngx_open_listening_sockets(ngx_cycle_t *cycle);
void ngx_configure_listening_sockets(ngx_cycle_t *cycle);
void ngx_close_listening_sockets(ngx_cycle_t *cycle);
void ngx_close_connection(ngx_connection_t *c);
ngx_int_t ngx_connection_local_sockaddr(ngx_connection_t *c, ngx_str_t *s,
    ngx_uint_t port);
ngx_int_t ngx_connection_error(ngx_connection_t *c, ngx_err_t err, char *text);

ngx_connection_t *ngx_get_connection(ngx_socket_t s, ngx_log_t *log);
void ngx_free_connection(ngx_connection_t *c);

void ngx_reusable_connection(ngx_connection_t *c, ngx_uint_t reusable);

#endif /* _NGX_CONNECTION_H_INCLUDED_ */
