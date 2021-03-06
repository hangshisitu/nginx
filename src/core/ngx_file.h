
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_FILE_H_INCLUDED_
#define _NGX_FILE_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>


struct ngx_file_s {
    ngx_fd_t                   fd;                 /* 文件描述符 */
    ngx_str_t                  name;               /* 文件路径 */
    ngx_file_info_t            info;               /* 文件信息 */

    off_t                      offset;             /* 文件偏移 */
    off_t                      sys_offset;         /* 系统偏移 */

    ngx_log_t                 *log;

#if (NGX_THREADS)
    ngx_int_t                (*thread_handler)(ngx_thread_task_t *task,
                                               ngx_file_t *file);
    void                      *thread_ctx;
#endif

#if (NGX_HAVE_FILE_AIO)
    ngx_event_aio_t           *aio;                /* 异步IO事件 */
#endif

    unsigned                   valid_info:1;       /*  */
    unsigned                   directio:1;         /*  */
};


#define NGX_MAX_PATH_LEVEL  3


typedef time_t (*ngx_path_manager_pt) (void *data);
typedef void (*ngx_path_loader_pt) (void *data);

/*
 * 解析配置文件中的路径的结构
 * 其中len ,level字段规定了在该路径name下生成临时文件时
 * 临时路径的产生算法
 */
typedef struct {
    ngx_str_t                  name;                /* 路径名 */
    size_t                     len;                 /* leavel规定的路径部分的长度 */
    size_t                     level[3];            /* 临时文件的三级目录大小 生成临时路径的hash算法hash因子 */

    ngx_path_manager_pt        manager;
    ngx_path_loader_pt         loader;
    void                      *data;

    u_char                    *conf_file;           /* 该路径的来源的配置文件 */
    ngx_uint_t                 line;                /* 该路径在来源的配置文件中的行数，主要用于记录日志，排查错误 */
} ngx_path_t;

/*
 * 用来生成临时文件的初始对象
 */
typedef struct {
    ngx_str_t                  name;               /* 临时路径的前缀字符 */
    size_t                     level[3];           /* 临时目录的三级目录大小，生成临时路径的hash因子 */
} ngx_path_init_t;

/*
 * 访问临时文件的结构
 */
typedef struct {
    ngx_file_t                 file;
    off_t                      offset;            /* 文件指针偏移 */
    ngx_path_t                *path;
    ngx_pool_t                *pool;
    char                      *warn;

    ngx_uint_t                 access;            /* 访问权限 */

    unsigned                   log_level:8;
    unsigned                   persistent:1;      /* 是否持久化 */
    unsigned                   clean:1;           /* 清理方式 */
} ngx_temp_file_t;

/* ngx_ext_rename_file 使用的更改文件名的控制对象 */
typedef struct {
    ngx_uint_t                 access;
    ngx_uint_t                 path_access;
    time_t                     time;
    ngx_fd_t                   fd;

    unsigned                   create_path:1;
    unsigned                   delete_file:1;

    ngx_log_t                 *log;
} ngx_ext_rename_file_t;

/* ngx_copy_file 使用的拷贝文件的控制对象 */
typedef struct {
    off_t                      size;         /* 拷贝大小 */    
    size_t                     buf_size;     /* 缓冲区大小 */

    ngx_uint_t                 access;       /* 目标文件的访问权限 */
    time_t                     time;         /* 目标文件的访问时间戳 */

    ngx_log_t                 *log;
} ngx_copy_file_t;


typedef struct ngx_tree_ctx_s  ngx_tree_ctx_t;

typedef ngx_int_t (*ngx_tree_init_handler_pt) (void *ctx, void *prev);
typedef ngx_int_t (*ngx_tree_handler_pt) (ngx_tree_ctx_t *ctx, ngx_str_t *name);

struct ngx_tree_ctx_s {
    off_t                      size;
    off_t                      fs_size;
    ngx_uint_t                 access;
    time_t                     mtime;

    ngx_tree_init_handler_pt   init_handler;
    ngx_tree_handler_pt        file_handler;
    ngx_tree_handler_pt        pre_tree_handler;
    ngx_tree_handler_pt        post_tree_handler;
    ngx_tree_handler_pt        spec_handler;

    void                      *data;
    size_t                     alloc;

    ngx_log_t                 *log;
};


ngx_int_t ngx_get_full_name(ngx_pool_t *pool, ngx_str_t *prefix,
    ngx_str_t *name);

ssize_t ngx_write_chain_to_temp_file(ngx_temp_file_t *tf, ngx_chain_t *chain);
ngx_int_t ngx_create_temp_file(ngx_file_t *file, ngx_path_t *path,
    ngx_pool_t *pool, ngx_uint_t persistent, ngx_uint_t clean,
    ngx_uint_t access);
void ngx_create_hashed_filename(ngx_path_t *path, u_char *file, size_t len);
ngx_int_t ngx_create_path(ngx_file_t *file, ngx_path_t *path);
ngx_err_t ngx_create_full_path(u_char *dir, ngx_uint_t access);
ngx_int_t ngx_add_path(ngx_conf_t *cf, ngx_path_t **slot);
ngx_int_t ngx_create_paths(ngx_cycle_t *cycle, ngx_uid_t user);
ngx_int_t ngx_ext_rename_file(ngx_str_t *src, ngx_str_t *to,
    ngx_ext_rename_file_t *ext);
ngx_int_t ngx_copy_file(u_char *from, u_char *to, ngx_copy_file_t *cf);
ngx_int_t ngx_walk_tree(ngx_tree_ctx_t *ctx, ngx_str_t *tree);

ngx_atomic_uint_t ngx_next_temp_number(ngx_uint_t collision);

char *ngx_conf_set_path_slot(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
char *ngx_conf_merge_path_value(ngx_conf_t *cf, ngx_path_t **path,
    ngx_path_t *prev, ngx_path_init_t *init);
char *ngx_conf_set_access_slot(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);


extern ngx_atomic_t      *ngx_temp_number;
extern ngx_atomic_int_t   ngx_random_number;


#endif /* _NGX_FILE_H_INCLUDED_ */
