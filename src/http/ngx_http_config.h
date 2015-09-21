
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_HTTP_CONFIG_H_INCLUDED_
#define _NGX_HTTP_CONFIG_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

/**
 * NGX_HTTP_MODULE类型模块的配置上下文结构体
 */
typedef struct {
    void        **main_conf;        /* 指针数组，数组元素是由create_main_conf创建 */
    void        **srv_conf;         /* 指针数组，数组元素是由create_srv_conf创建 */
    void        **loc_conf;         /* 指针数组，数组元素是由create_loc_conf创建 */
} ngx_http_conf_ctx_t;

/**
 * NGX_HTTP_MODULE类型模块的配置上下文生成器的结构
 * 提供解析配置项时各个阶段的钩子函数
 */
typedef struct {
    ngx_int_t   (*preconfiguration)(ngx_conf_t *cf);                          /* 解析配置文件前调用 */
    ngx_int_t   (*postconfiguration)(ngx_conf_t *cf);                         /* 配置文件解析完成后调用 */

    void       *(*create_main_conf)(ngx_conf_t *cf);                          /* 创建直隶于http块的配置项结构体 */
    char       *(*init_main_conf)(ngx_conf_t *cf, void *conf);                /* 初始化直隶与http块的配置项结构体 */

    void       *(*create_srv_conf)(ngx_conf_t *cf);                           /* 创建直隶于server块的配置项结构体 */
    char       *(*merge_srv_conf)(ngx_conf_t *cf, void *prev, void *conf);    /* 合并 http和server下的同名配置项 */

    void       *(*create_loc_conf)(ngx_conf_t *cf);                           /* 创建直隶于location 块的配置项结构体 */
    char       *(*merge_loc_conf)(ngx_conf_t *cf, void *prev, void *conf);    /* 合并 server 和location块的同名配置项 */
} ngx_http_module_t;


#define NGX_HTTP_MODULE           0x50545448   /* "HTTP" */

#define NGX_HTTP_MAIN_CONF        0x02000000
#define NGX_HTTP_SRV_CONF         0x04000000
#define NGX_HTTP_LOC_CONF         0x08000000
#define NGX_HTTP_UPS_CONF         0x10000000
#define NGX_HTTP_SIF_CONF         0x20000000
#define NGX_HTTP_LIF_CONF         0x40000000
#define NGX_HTTP_LMT_CONF         0x80000000


#define NGX_HTTP_MAIN_CONF_OFFSET  offsetof(ngx_http_conf_ctx_t, main_conf)
#define NGX_HTTP_SRV_CONF_OFFSET   offsetof(ngx_http_conf_ctx_t, srv_conf)
#define NGX_HTTP_LOC_CONF_OFFSET   offsetof(ngx_http_conf_ctx_t, loc_conf)


#define ngx_http_get_module_main_conf(r, module)                             \
    (r)->main_conf[module.ctx_index]
#define ngx_http_get_module_srv_conf(r, module)  (r)->srv_conf[module.ctx_index]
#define ngx_http_get_module_loc_conf(r, module)  (r)->loc_conf[module.ctx_index]


#define ngx_http_conf_get_module_main_conf(cf, module)                        \
    ((ngx_http_conf_ctx_t *) cf->ctx)->main_conf[module.ctx_index]
#define ngx_http_conf_get_module_srv_conf(cf, module)                         \
    ((ngx_http_conf_ctx_t *) cf->ctx)->srv_conf[module.ctx_index]
#define ngx_http_conf_get_module_loc_conf(cf, module)                         \
    ((ngx_http_conf_ctx_t *) cf->ctx)->loc_conf[module.ctx_index]

#define ngx_http_cycle_get_module_main_conf(cycle, module)                    \
    (cycle->conf_ctx[ngx_http_module.index] ?                                 \
        ((ngx_http_conf_ctx_t *) cycle->conf_ctx[ngx_http_module.index])      \
            ->main_conf[module.ctx_index]:                                    \
        NULL)


#endif /* _NGX_HTTP_CONFIG_H_INCLUDED_ */
