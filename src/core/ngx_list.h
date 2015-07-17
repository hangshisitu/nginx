
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_LIST_H_INCLUDED_
#define _NGX_LIST_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>


typedef struct ngx_list_part_s  ngx_list_part_t;


struct ngx_list_part_s {
    void             *elts;                        //节点空间地址
    ngx_uint_t        nelts;                    //该part 已使用的节点数
    ngx_list_part_t  *next;                   //指向下一个part
};

//一个链表由多个ngx_list_part_s组成
//part为链表的第一个ngx_list_part_s
//last 指向链表的最后一个ngx_list_part_s
typedef struct {
    ngx_list_part_t  *last;                   //指向链表的最后一个ngx_list_part_s
    ngx_list_part_t   part;                   //链表的第一个ngx_list_part_s
    size_t            size;                       //链表节点大小
    ngx_uint_t        nalloc;                 //ngx_list_part_s  可容纳的节点数
    ngx_pool_t       *pool;                  //链表所在内存池
} ngx_list_t;


ngx_list_t *ngx_list_create(ngx_pool_t *pool, ngx_uint_t n, size_t size);

static ngx_inline ngx_int_t
ngx_list_init(ngx_list_t *list, ngx_pool_t *pool, ngx_uint_t n, size_t size)
{
    list->part.elts = ngx_palloc(pool, n * size);
    if (list->part.elts == NULL) {
        return NGX_ERROR;
    }

    list->part.nelts = 0;
    list->part.next = NULL;
    list->last = &list->part;
    list->size = size;
    list->nalloc = n;
    list->pool = pool;

    return NGX_OK;
}


/*
 *
 *  the iteration through the list:
 *
 *  part = &list.part;
 *  data = part->elts;
 *
 *  for (i = 0 ;; i++) {
 *
 *      if (i >= part->nelts) {
 *          if (part->next == NULL) {
 *              break;
 *          }
 *
 *          part = part->next;
 *          data = part->elts;
 *          i = 0;
 *      }
 *
 *      ...  data[i] ...
 *
 *  }
 */


void *ngx_list_push(ngx_list_t *list);


#endif /* _NGX_LIST_H_INCLUDED_ */
