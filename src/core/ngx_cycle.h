
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_CYCLE_H_INCLUDED_
#define _NGX_CYCLE_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>


#ifndef NGX_CYCLE_POOL_SIZE
#define NGX_CYCLE_POOL_SIZE     NGX_DEFAULT_POOL_SIZE
#endif


#define NGX_DEBUG_POINTS_STOP   1
#define NGX_DEBUG_POINTS_ABORT  2


typedef struct ngx_shm_zone_s  ngx_shm_zone_t;

typedef ngx_int_t (*ngx_shm_zone_init_pt) (ngx_shm_zone_t *zone, void *data);

struct ngx_shm_zone_s {
    void                     *data;
    ngx_shm_t                 shm;
    ngx_shm_zone_init_pt      init;
    void                     *tag;
    ngx_uint_t                noreuse;  /* unsigned  noreuse:1; */
};


struct ngx_cycle_s {

    /*
     * conf_ctx为一个数组，数组元素的类型为void*，元素个数为ngx_max_module模块数。
     * ngx_module_s的index字段指示了该模块的配置结构信息在conf_ctx数组中的下标。
     * 不同模块的配置信息结构是不同
     * 例如 ngx_core_module 的配置信息结构为ngx_core_conf_t
     *      ngx_errlog_module 的配置信息无需保存因此其对应的槽位空闲不用
     *      ngx_events_module 的配置信息结构为一个数组，数组元素指向为各子模块的配置结构
     */
    void                  ****conf_ctx;

	/**
	* 随ngx_cycle_t同生命周期的内存池，是生命周期最长的，除非nginx关闭或重启。内存块大小
	* 由NGX_CYCLE_POOL_SIZE定义，是16KB。
	*/
    ngx_pool_t               *pool;

    ngx_log_t                *log;
    ngx_log_t                 new_log;

    ngx_uint_t                log_use_stderr;  /* unsigned  log_use_stderr:1; */

    ngx_connection_t        **files;                    // 连接对象数组
    ngx_connection_t         *free_connections;         // 空闲链接数组
    ngx_uint_t                free_connection_n;        // 空闲链接个数

    ngx_queue_t               reusable_connections_queue;   // 重用链接队列

	/**
	* 监听套接字的数组，类型是ngx_listening_t，ngx_create_listening函数会向这个数组
	* 添加新的监听句柄。
	*/
    ngx_array_t               listening;
    ngx_array_t               paths;                    // 路径数组，ngx_path_t

	/**
	* 所有打开的文件描述符的链表，ngx_open_file_t类型。ngx_conf_file.c中的
	*      ngx_conf_open_file(ngx_cycyle_t *cycle, ngx_str_t *name)
	* 可以返回名字为name的打开的文件描述符，如果对应的描述符不存在，则打开并存入open_files中。
	* 比如：logs/error.log的文件描述符就在其中。
    * 链表保存在ngx_init_cycle中要打开的文件
	*/
    ngx_list_t                open_files;

	/**
	* 共享内存的链表，ngx_shm_zone_t类型。nginx会把需要进程共享的数据放在共享内存中。
	* 比如，accept锁就放在这里。worker进程只有获取这个锁之后，才能accept到新的连接。
	* 防止惊群。
	*/
    ngx_list_t                shared_memory;

    ngx_uint_t                connection_n;       // 链接个数  对应配置指令 main.events.worker_connections 
    ngx_uint_t                files_n;            // 打开文件个数 

	/**
	* 连接池，nginx把连接池组织成综合数组和链表特性的一种数据结构，后文会详细介绍。
	*/
    ngx_connection_t         *connections;
    ngx_event_t              *read_events;       // 读事件
    ngx_event_t              *write_events;      // 写事件

    ngx_cycle_t              *old_cycle;         //调用 ngx_init_cycle 是传入的参数

    ngx_str_t                 conf_file;         // 配置文件名 
    ngx_str_t                 conf_param;        // 由命令行-g提供配置参数
    ngx_str_t                 conf_prefix;       // 配置前缀 
    ngx_str_t                 prefix;            // nginx所在路径
    ngx_str_t                 lock_file;         
    ngx_str_t                 hostname;          // 主机名
};

/**
* ngx_core_module 的配置结构
*/
typedef struct {
     ngx_flag_t               daemon;
     ngx_flag_t               master;

     ngx_msec_t               timer_resolution;

     ngx_int_t                worker_processes;
     ngx_int_t                debug_points;

     ngx_int_t                rlimit_nofile;
     off_t                    rlimit_core;

     int                      priority;

     ngx_uint_t               cpu_affinity_n;
     uint64_t                *cpu_affinity;

     char                    *username;
     ngx_uid_t                user;
     ngx_gid_t                group;

     ngx_str_t                working_directory;
     ngx_str_t                lock_file;

     ngx_str_t                pid;
     ngx_str_t                oldpid;

     ngx_array_t              env;
     char                   **environment;
} ngx_core_conf_t;


#define ngx_is_init_cycle(cycle)  (cycle->conf_ctx == NULL)


ngx_cycle_t *ngx_init_cycle(ngx_cycle_t *old_cycle);
ngx_int_t ngx_create_pidfile(ngx_str_t *name, ngx_log_t *log);
void ngx_delete_pidfile(ngx_cycle_t *cycle);
ngx_int_t ngx_signal_process(ngx_cycle_t *cycle, char *sig);
void ngx_reopen_files(ngx_cycle_t *cycle, ngx_uid_t user);
char **ngx_set_environment(ngx_cycle_t *cycle, ngx_uint_t *last);
ngx_pid_t ngx_exec_new_binary(ngx_cycle_t *cycle, char *const *argv);
uint64_t ngx_get_cpu_affinity(ngx_uint_t n);
ngx_shm_zone_t *ngx_shared_memory_add(ngx_conf_t *cf, ngx_str_t *name,
    size_t size, void *tag);


extern volatile ngx_cycle_t  *ngx_cycle;
extern ngx_array_t            ngx_old_cycles;
extern ngx_module_t           ngx_core_module;
extern ngx_uint_t             ngx_test_config;
extern ngx_uint_t             ngx_quiet_mode;


#endif /* _NGX_CYCLE_H_INCLUDED_ */
