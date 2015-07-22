
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 * 可自旋等待的锁
 */


#ifndef _NGX_SHMTX_H_INCLUDED_
#define _NGX_SHMTX_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>

/* 锁使用的原子操作变量 */
typedef struct {
    ngx_atomic_t   lock;
#if (NGX_HAVE_POSIX_SEM)
    ngx_atomic_t   wait;
#endif
} ngx_shmtx_sh_t;


typedef struct {
#if (NGX_HAVE_ATOMIC_OPS)
    ngx_atomic_t  *lock;
#if (NGX_HAVE_POSIX_SEM)
    ngx_atomic_t  *wait;            /* 等待计数 信号量启用时使用 */
    ngx_uint_t     semaphore;       /* 标记是否启用了信号量 */
    sem_t          sem;             /* 信号量 */
#endif
#else
    ngx_fd_t       fd;
    u_char        *name;
#endif
    ngx_uint_t     spin;            /* 自旋等待次数 */
} ngx_shmtx_t;


ngx_int_t ngx_shmtx_create(ngx_shmtx_t *mtx, ngx_shmtx_sh_t *addr,
    u_char *name);
void ngx_shmtx_destroy(ngx_shmtx_t *mtx);
ngx_uint_t ngx_shmtx_trylock(ngx_shmtx_t *mtx);
void ngx_shmtx_lock(ngx_shmtx_t *mtx);
void ngx_shmtx_unlock(ngx_shmtx_t *mtx);
ngx_uint_t ngx_shmtx_force_unlock(ngx_shmtx_t *mtx, ngx_pid_t pid);


#endif /* _NGX_SHMTX_H_INCLUDED_ */
