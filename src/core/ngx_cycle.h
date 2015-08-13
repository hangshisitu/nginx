
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
     * conf_ctxΪһ�����飬����Ԫ�ص�����Ϊvoid*��Ԫ�ظ���Ϊngx_max_moduleģ������
     * ngx_module_s��index�ֶ�ָʾ�˸�ģ������ýṹ��Ϣ��conf_ctx�����е��±ꡣ
     * ��ͬģ���������Ϣ�ṹ�ǲ�ͬ
     * ���� ngx_core_module ��������Ϣ�ṹΪngx_core_conf_t
     *      ngx_errlog_module ��������Ϣ���豣��������Ӧ�Ĳ�λ���в���
     *      ngx_events_module ��������Ϣ�ṹΪһ�����飬����Ԫ��ָ��Ϊ����ģ������ýṹ
     */
    void                  ****conf_ctx;

	/**
	* ��ngx_cycle_tͬ�������ڵ��ڴ�أ�������������ģ�����nginx�رջ��������ڴ���С
	* ��NGX_CYCLE_POOL_SIZE���壬��16KB��
	*/
    ngx_pool_t               *pool;

    ngx_log_t                *log;
    ngx_log_t                 new_log;

    ngx_uint_t                log_use_stderr;  /* unsigned  log_use_stderr:1; */

    ngx_connection_t        **files;                    // ���Ӷ�������
    ngx_connection_t         *free_connections;         // ������������
    ngx_uint_t                free_connection_n;        // �������Ӹ���

    ngx_queue_t               reusable_connections_queue;   // �������Ӷ���

	/**
	* �����׽��ֵ����飬������ngx_listening_t��ngx_create_listening���������������
	* ����µļ��������
	*/
    ngx_array_t               listening;
    ngx_array_t               paths;                    // ·�����飬ngx_path_t

	/**
	* ���д򿪵��ļ�������������ngx_open_file_t���͡�ngx_conf_file.c�е�
	*      ngx_conf_open_file(ngx_cycyle_t *cycle, ngx_str_t *name)
	* ���Է�������Ϊname�Ĵ򿪵��ļ��������������Ӧ�������������ڣ���򿪲�����open_files�С�
	* ���磺logs/error.log���ļ��������������С�
    * ��������ngx_init_cycle��Ҫ�򿪵��ļ�
	*/
    ngx_list_t                open_files;

	/**
	* �����ڴ������ngx_shm_zone_t���͡�nginx�����Ҫ���̹�������ݷ��ڹ����ڴ��С�
	* ���磬accept���ͷ������worker����ֻ�л�ȡ�����֮�󣬲���accept���µ����ӡ�
	* ��ֹ��Ⱥ��
	*/
    ngx_list_t                shared_memory;

    ngx_uint_t                connection_n;       // ���Ӹ���  ��Ӧ����ָ�� main.events.worker_connections 
    ngx_uint_t                files_n;            // ���ļ����� 

	/**
	* ���ӳأ�nginx�����ӳ���֯���ۺ�������������Ե�һ�����ݽṹ�����Ļ���ϸ���ܡ�
	*/
    ngx_connection_t         *connections;
    ngx_event_t              *read_events;       // ���¼�
    ngx_event_t              *write_events;      // д�¼�

    ngx_cycle_t              *old_cycle;         //���� ngx_init_cycle �Ǵ���Ĳ���

    ngx_str_t                 conf_file;         // �����ļ��� 
    ngx_str_t                 conf_param;        // ��������-g�ṩ���ò���
    ngx_str_t                 conf_prefix;       // ����ǰ׺ 
    ngx_str_t                 prefix;            // nginx����·��
    ngx_str_t                 lock_file;         
    ngx_str_t                 hostname;          // ������
};

/**
* ngx_core_module �����ýṹ
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
