
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_PALLOC_H_INCLUDED_
#define _NGX_PALLOC_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>


/*
 * NGX_MAX_ALLOC_FROM_POOL should be (ngx_pagesize - 1), i.e. 4095 on x86.
 * On Windows NT it decreases a number of locked pages in a kernel.
 */
#define NGX_MAX_ALLOC_FROM_POOL  (ngx_pagesize - 1)

#define NGX_DEFAULT_POOL_SIZE    (16 * 1024)

#define NGX_POOL_ALIGNMENT       16
#define NGX_MIN_POOL_SIZE                                                     \
    ngx_align((sizeof(ngx_pool_t) + 2 * sizeof(ngx_pool_large_t)),            \
              NGX_POOL_ALIGNMENT)


typedef void (*ngx_pool_cleanup_pt)(void *data);

typedef struct ngx_pool_cleanup_s  ngx_pool_cleanup_t;
/* �ڴ��������������ڴ��ڵ� */
struct ngx_pool_cleanup_s {
    ngx_pool_cleanup_pt   handler;      /* �ڴ��������� */
    void                 *data;         /* ָ����ڴ���з���Ŀռ� */
    ngx_pool_cleanup_t   *next;         /* ָ����һ�ڵ� */
};


typedef struct ngx_pool_large_s  ngx_pool_large_t;

/*
 * �ڴ���д��ڴ��ڵ�
 * �ýڵ�ά�����ڴ�û�н���Ԥ��������������ʱ������
 */
struct ngx_pool_large_s {
    ngx_pool_large_t     *next;              /* ָ����һ���ڵ� */
    void                 *alloc;             /* ָ�������ڴ�ռ� */
};

/* �ڴ����Ԥ������ڴ�ڵ� */
typedef struct {
    u_char               *last;              /* ָ����ÿռ���ʼλ�� */
    u_char               *end;               /* ָ����ÿռ����λ�� */
    ngx_pool_t           *next;              /* ָ����һ�� ngx_pool_data_t �ڵ� */
    ngx_uint_t            failed;            /* �ռ�����ʧ�ܴ��� */
} ngx_pool_data_t;


struct ngx_pool_s {
    ngx_pool_data_t       d;                 /*  */
    size_t                max;               /* �ڴ���ܷ��������ڴ��С*/
    ngx_pool_t           *current;           /* ָ�� ngx_pool_data_t �����е�һ��ʧ�ܼ���û�г���3�Ľڵ�*/
    ngx_chain_t          *chain;
    ngx_pool_large_t     *large;             /* ά��һ�� ngx_pool_large_t ������ */
    ngx_pool_cleanup_t   *cleanup;           /* ָ���������ڴ�� */
    ngx_log_t            *log;               /* ��־���� */
};

/* �ڴ���������ļ��ṹ */
typedef struct {
    ngx_fd_t              fd;                /* �ļ������� */
    u_char               *name;              /* �ļ��� */
    ngx_log_t            *log;               /* ��־���� */
} ngx_pool_cleanup_file_t;


void *ngx_alloc(size_t size, ngx_log_t *log);
void *ngx_calloc(size_t size, ngx_log_t *log);

ngx_pool_t *ngx_create_pool(size_t size, ngx_log_t *log);
void ngx_destroy_pool(ngx_pool_t *pool);
void ngx_reset_pool(ngx_pool_t *pool);

void *ngx_palloc(ngx_pool_t *pool, size_t size);
void *ngx_pnalloc(ngx_pool_t *pool, size_t size);
void *ngx_pcalloc(ngx_pool_t *pool, size_t size);
void *ngx_pmemalign(ngx_pool_t *pool, size_t size, size_t alignment);
ngx_int_t ngx_pfree(ngx_pool_t *pool, void *p);


ngx_pool_cleanup_t *ngx_pool_cleanup_add(ngx_pool_t *p, size_t size);
void ngx_pool_run_cleanup_file(ngx_pool_t *p, ngx_fd_t fd);
void ngx_pool_cleanup_file(void *data);
void ngx_pool_delete_file(void *data);


#endif /* _NGX_PALLOC_H_INCLUDED_ */
