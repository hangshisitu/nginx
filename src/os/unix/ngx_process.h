
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_PROCESS_H_INCLUDED_
#define _NGX_PROCESS_H_INCLUDED_


#include <ngx_setaffinity.h>
#include <ngx_setproctitle.h>


typedef pid_t       ngx_pid_t;

#define NGX_INVALID_PID  -1

typedef void (*ngx_spawn_proc_pt) (ngx_cycle_t *cycle, void *data);

typedef struct {
    ngx_pid_t           pid;                              //����ID
    int                 status;                           //����״̬

    /* channel[0] ��  channel[1] ��һ�������ӵ��׽��ֶԣ����ɶ���д
     * work������channel[1]�ϼ������¼����յ�NGX_CMD_OPEN_CHANNEL ����,�ͽ�������һ�е�fd ����������slot���̵�channel[0]
     * master��work���̷�������ʹ��channel[0]
     */
    ngx_socket_t        channel[2];                       

    ngx_spawn_proc_pt   proc;                             //������ѭ������
    void               *data;                             //proc�Ĳ���
    char               *name;                             //������

    unsigned            respawn:1;
    unsigned            just_spawn:1;
    unsigned            detached:1;
    unsigned            exiting:1;                        //��ǽ��������˳�
    unsigned            exited:1;                         //��ǽ������˳�
} ngx_process_t;

/* ִ�ж������ļ��������Ľṹ */
typedef struct {
    char         *path;
    char         *name;
    char *const  *argv;
    char *const  *envp;
} ngx_exec_ctx_t;


#define NGX_MAX_PROCESSES         1024

#define NGX_PROCESS_NORESPAWN     -1       /* �ӽ����˳�ʱ,�����̲����ٴδ���, �ñ�����ڴ��� "cache loader process". */
/* �� nginx - s reload ʱ, �������δ���ص� proxy_cache_path, ����Ҫ�ٴδ���
 * "cache loader process"����, ���� NGX_PROCESS_JUST_SPAWN������������Ǻ�, 
 * ��ֹ "master�����ϵ�worker����,�ϵ�cache manager����,�ϵ�cache loader����
 * (�������)����NGX_CMD_QUIT��SIGQUIT" ʱ, ����Ϊ����������ϵ�cache loader����*/
#define NGX_PROCESS_JUST_SPAWN    -2
#define NGX_PROCESS_RESPAWN       -3       /* �ӽ����쳣�˳�ʱ,master�����´�����, �統worker��cache manager�쳣�˳�ʱ,�����̻����´����� */

/* �� nginx - s reload ʱ, master�����ϵ�worker����, �ϵ�cache manager����, ��
 * ��cache loader����(�������)���� ngx_write_channel(NGX_CMD_QUIT)(���ʧ����
 * ����SIGQUIT�ź�);�ñ��������ǽ�����������Щ���´������ӽ���; �����ľ����ϵ��ӽ���.*/
#define NGX_PROCESS_JUST_RESPAWN  -4
#define NGX_PROCESS_DETACHED      -5       /* �ȴ����滻 */


#define ngx_getpid   getpid

#ifndef ngx_log_pid
#define ngx_log_pid  ngx_pid
#endif


ngx_pid_t ngx_spawn_process(ngx_cycle_t *cycle,
    ngx_spawn_proc_pt proc, void *data, char *name, ngx_int_t respawn);
ngx_pid_t ngx_execute(ngx_cycle_t *cycle, ngx_exec_ctx_t *ctx);
ngx_int_t ngx_init_signals(ngx_log_t *log);
void ngx_debug_point(void);


#if (NGX_HAVE_SCHED_YIELD)
#define ngx_sched_yield()  sched_yield()
#else
#define ngx_sched_yield()  usleep(1)
#endif


extern int            ngx_argc;
extern char         **ngx_argv;
extern char         **ngx_os_argv;

extern ngx_pid_t      ngx_pid;
extern ngx_socket_t   ngx_channel;
extern ngx_int_t      ngx_process_slot;
extern ngx_int_t      ngx_last_process;
extern ngx_process_t  ngx_processes[NGX_MAX_PROCESSES];


#endif /* _NGX_PROCESS_H_INCLUDED_ */
