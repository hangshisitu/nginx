
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_CONF_FILE_H_INCLUDED_
#define _NGX_CONF_FILE_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>


/*
 *        AAAA  number of arguments
 *      FF      command flags
 *    TT        command type, i.e. HTTP "location" or "server" command
 */

#define NGX_CONF_NOARGS      0x00000001
#define NGX_CONF_TAKE1       0x00000002
#define NGX_CONF_TAKE2       0x00000004
#define NGX_CONF_TAKE3       0x00000008
#define NGX_CONF_TAKE4       0x00000010
#define NGX_CONF_TAKE5       0x00000020
#define NGX_CONF_TAKE6       0x00000040
#define NGX_CONF_TAKE7       0x00000080

#define NGX_CONF_MAX_ARGS    8

#define NGX_CONF_TAKE12      (NGX_CONF_TAKE1|NGX_CONF_TAKE2)
#define NGX_CONF_TAKE13      (NGX_CONF_TAKE1|NGX_CONF_TAKE3)

#define NGX_CONF_TAKE23      (NGX_CONF_TAKE2|NGX_CONF_TAKE3)

#define NGX_CONF_TAKE123     (NGX_CONF_TAKE1|NGX_CONF_TAKE2|NGX_CONF_TAKE3)
#define NGX_CONF_TAKE1234    (NGX_CONF_TAKE1|NGX_CONF_TAKE2|NGX_CONF_TAKE3   \
                              |NGX_CONF_TAKE4)

#define NGX_CONF_ARGS_NUMBER 0x000000ff
#define NGX_CONF_BLOCK       0x00000100
#define NGX_CONF_FLAG        0x00000200  /* 配置指令只有一个参数 */
#define NGX_CONF_ANY         0x00000400  /* 配置指令的参数不做限制 */
#define NGX_CONF_1MORE       0x00000800  /* 配置指令有一个以上的参数 */
#define NGX_CONF_2MORE       0x00001000  /* 配置指令有两个以上的参数 */
#define NGX_CONF_MULTI       0x00000000  /* compatibility */

#define NGX_DIRECT_CONF      0x00010000  /* 直接配置指令*/

#define NGX_MAIN_CONF        0x01000000  /* 主配置指令 */
#define NGX_ANY_CONF         0x0F000000



#define NGX_CONF_UNSET       -1
#define NGX_CONF_UNSET_UINT  (ngx_uint_t) -1
#define NGX_CONF_UNSET_PTR   (void *) -1
#define NGX_CONF_UNSET_SIZE  (size_t) -1
#define NGX_CONF_UNSET_MSEC  (ngx_msec_t) -1


#define NGX_CONF_OK          NULL
#define NGX_CONF_ERROR       (void *) -1

#define NGX_CONF_BLOCK_START 1
#define NGX_CONF_BLOCK_DONE  2
#define NGX_CONF_FILE_DONE   3

#define NGX_CORE_MODULE      0x45524F43  /* "CORE" */ /* 该类型的模块处理配置文件中指令Context为main 的指令 */
#define NGX_CONF_MODULE      0x464E4F43  /* "CONF" */ 


#define NGX_MAX_CONF_ERRSTR  1024

/**
* module的配置指令
*/
struct ngx_command_s {
	/**
	* 指令名，与配置文件中一致
	*/
    ngx_str_t             name;

	/**
	* 指令的类型，以及参数的个数。这个属性有两个作用：
	*  1. 实现只解析某个类型的指令，比如当前这个指令是event module类型的，而正在解析的是
	*     http module，所以会跳过所有不是http module类型的指令。
	*  2. 实现指令参数个数的校验。
	*/
    ngx_uint_t            type;

	/*
	* 回调函数，在解析配置文件时，遇到这个指令时调用。
	* cf: 包括配置参数信息cf->args（ngx_array_t类型），以及指令对应的模块上下文cf->ctx
	*      在解析不同模块的指令时，这个上下文信息不同。比如在解析core module时，cf->ctx
	*      是ngx_cycle->conf_ctx也就是所有core module的配置结构数组，而在解析http module
	*      时cf->ctx是ngx_http_conf_ctx_t类型的，其中包含所有http module的main、srv、loc
	*      的配置结构数组。
	* cmd: 指令对应的ngx_command_t结构。
	* conf：指令对应的模块的配置信息。
	*/
    char               *(*set)(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);

	/**
	* 对http module有效，http module的配置结构信息（main、srv、loc）都存放在ngx_http_conf_ctx_t
	* 中对应的数组，conf属性指示这个指令的配置结构是main、srv还是loc。
	*/
    ngx_uint_t            conf;

	/**
	* 指令对应属性在模块配置结构中的偏移量。
	*/
    ngx_uint_t            offset;

	/**
	* 一般是函数指针，在set回调函数中调用。
	*/
    void                 *post;
};

#define ngx_null_command  { ngx_null_string, 0, NULL, 0, 0, NULL }


struct ngx_open_file_s {
    ngx_fd_t              fd;
    ngx_str_t             name;

    void                (*flush)(ngx_open_file_t *file, ngx_log_t *log);
    void                 *data;
};


#define NGX_MODULE_V1          0, 0, 0, 0, 0, 0, 1
#define NGX_MODULE_V1_PADDING  0, 0, 0, 0, 0, 0, 0, 0

struct ngx_module_s {
	
	/**
	* 在具体类型模块（http、event等）的全局配置结构数组的下标。以http module模块为例，
	* nginx把所有的http module的config信息存放在ngx_http_conf_ctx_t类型的变量中，
	* 这个变量只有3个属性，分别是所有http module的main、srv、loc的config信息的数组。
	* 如果该模块是http module，则ctx_index是该模块的config信息（main、srv、loc）
	* 在ngx_http_conf_ctx_t中的下标。
	*/
    ngx_uint_t            ctx_index;

	/**
	* nginx把所有模块（ngx_module_t）存放到ngx_modules数组中，这个数组在nginx源码路
	* 径的objs/ngx_modules.c中，是在运行configure脚本后生成的。index属性就是该模块
	* 在ngx_modules数组中的下标。同时nginx把所有的core module的配置结构存放到ngx_cycle的
	* conf_ctx数组中，index也是该模块的配置结构在ngx_cycle->conf_ctx数组中的下标。
	*/
    ngx_uint_t            index;

    ngx_uint_t            spare0;
    ngx_uint_t            spare1;
    ngx_uint_t            spare2;
    ngx_uint_t            spare3;

    ngx_uint_t            version;

	/**
	* 模块的上下文属性，同一类型的模块的属性是相同的，比如core module的ctx是ngx_core_module_t类型。
	* 而http module的ctx是ngx_http_moduel_t类型，event module的ctx是ngx_event_module_t类型等等。
	* 相应类型的模块由分开处理的，比如所有的http module由ngx_http_module解析处理，而所有的event module
	* 由ngx_events_module解析处理。
	*/
    void                 *ctx;

	/**
	* 该模块支持的指令的数组，最后以一个空指令结尾。ngx_commond_t的分析见下文。
	*/
    ngx_command_t        *commands;

	/**
	* 模块的类型，nginx所有的模块类型：
	*      NGX_CORE_MODULE
	*      NGX_CONF_MODULE
	*      NGX_HTTP_MODULE
	*      NGX_EVENT_MODULE
	*      NGX_MAIL_MODULE
	*	   NGX_STREAM_MODULE
	* 这些不同的类型也指定了不同的ctx。
	*/
    ngx_uint_t            type;

	/* 接下来都是一些回调函数，在nginx初始化过程的特定时间点调用 */
    ngx_int_t           (*init_master)(ngx_log_t *log);

	/* 初始化完所有模块后调用，在ngx_int_cycle函数（ngx_cycle.c）中 */
    ngx_int_t           (*init_module)(ngx_cycle_t *cycle);

    ngx_int_t           (*init_process)(ngx_cycle_t *cycle);
    ngx_int_t           (*init_thread)(ngx_cycle_t *cycle);
    void                (*exit_thread)(ngx_cycle_t *cycle);
    void                (*exit_process)(ngx_cycle_t *cycle);

    void                (*exit_master)(ngx_cycle_t *cycle);

    uintptr_t             spare_hook0;
    uintptr_t             spare_hook1;
    uintptr_t             spare_hook2;
    uintptr_t             spare_hook3;
    uintptr_t             spare_hook4;
    uintptr_t             spare_hook5;
    uintptr_t             spare_hook6;
    uintptr_t             spare_hook7;
};

/*NGX_CORE_MODULE 类型模块的上下文结构*/
typedef struct {
    ngx_str_t             name;
	void               *(*create_conf)(ngx_cycle_t *cycle);              /* 创建模块配置信息结构*/
    char               *(*init_conf)(ngx_cycle_t *cycle, void *conf);    /* 初始化模块配置信息结构*/
} ngx_core_module_t;

/*
 * 配置文件结构
 */
typedef struct {
    ngx_file_t            file;                    /* 指向配置文件 */
    ngx_buf_t            *buffer;                  /* 配置命令缓冲区 */
    ngx_uint_t            line;                    /* 行号 */
} ngx_conf_file_t;


typedef char *(*ngx_conf_handler_pt)(ngx_conf_t *cf,
    ngx_command_t *dummy, void *conf);

/* 配置信息结构 */
struct ngx_conf_s {
    char                 *name;
    ngx_array_t          *args;                    /* 保存从配置文件读到的token */

    ngx_cycle_t          *cycle;
    ngx_pool_t           *pool;
    ngx_pool_t           *temp_pool;
    ngx_conf_file_t      *conf_file;               /* 指向待解析的配置文件*/
    ngx_log_t            *log;

    void                 *ctx;                     /* 配置上下文 */
    ngx_uint_t            module_type;             /* 模块类型：NGX_CORE_MODULE, ... */
    ngx_uint_t            cmd_type;                /* 配置命令类型：NGX_MAIN_CONF, ... */

    ngx_conf_handler_pt   handler;
    char                 *handler_conf;
};


typedef char *(*ngx_conf_post_handler_pt) (ngx_conf_t *cf,
    void *data, void *conf);

typedef struct {
    ngx_conf_post_handler_pt  post_handler;
} ngx_conf_post_t;


typedef struct {
    ngx_conf_post_handler_pt  post_handler;
    char                     *old_name;
    char                     *new_name;
} ngx_conf_deprecated_t;


typedef struct {
    ngx_conf_post_handler_pt  post_handler;
    ngx_int_t                 low;
    ngx_int_t                 high;
} ngx_conf_num_bounds_t;


typedef struct {
    ngx_str_t                 name;
    ngx_uint_t                value;
} ngx_conf_enum_t;


#define NGX_CONF_BITMASK_SET  1

typedef struct {
    ngx_str_t                 name;
    ngx_uint_t                mask;
} ngx_conf_bitmask_t;



char * ngx_conf_deprecated(ngx_conf_t *cf, void *post, void *data);
char *ngx_conf_check_num_bounds(ngx_conf_t *cf, void *post, void *data);


#define ngx_get_conf(conf_ctx, module)  conf_ctx[module.index]



#define ngx_conf_init_value(conf, default)                                   \
    if (conf == NGX_CONF_UNSET) {                                            \
        conf = default;                                                      \
    }

#define ngx_conf_init_ptr_value(conf, default)                               \
    if (conf == NGX_CONF_UNSET_PTR) {                                        \
        conf = default;                                                      \
    }

#define ngx_conf_init_uint_value(conf, default)                              \
    if (conf == NGX_CONF_UNSET_UINT) {                                       \
        conf = default;                                                      \
    }

#define ngx_conf_init_size_value(conf, default)                              \
    if (conf == NGX_CONF_UNSET_SIZE) {                                       \
        conf = default;                                                      \
    }

#define ngx_conf_init_msec_value(conf, default)                              \
    if (conf == NGX_CONF_UNSET_MSEC) {                                       \
        conf = default;                                                      \
    }

#define ngx_conf_merge_value(conf, prev, default)                            \
    if (conf == NGX_CONF_UNSET) {                                            \
        conf = (prev == NGX_CONF_UNSET) ? default : prev;                    \
    }

#define ngx_conf_merge_ptr_value(conf, prev, default)                        \
    if (conf == NGX_CONF_UNSET_PTR) {                                        \
        conf = (prev == NGX_CONF_UNSET_PTR) ? default : prev;                \
    }

#define ngx_conf_merge_uint_value(conf, prev, default)                       \
    if (conf == NGX_CONF_UNSET_UINT) {                                       \
        conf = (prev == NGX_CONF_UNSET_UINT) ? default : prev;               \
    }

#define ngx_conf_merge_msec_value(conf, prev, default)                       \
    if (conf == NGX_CONF_UNSET_MSEC) {                                       \
        conf = (prev == NGX_CONF_UNSET_MSEC) ? default : prev;               \
    }

#define ngx_conf_merge_sec_value(conf, prev, default)                        \
    if (conf == NGX_CONF_UNSET) {                                            \
        conf = (prev == NGX_CONF_UNSET) ? default : prev;                    \
    }

#define ngx_conf_merge_size_value(conf, prev, default)                       \
    if (conf == NGX_CONF_UNSET_SIZE) {                                       \
        conf = (prev == NGX_CONF_UNSET_SIZE) ? default : prev;               \
    }

#define ngx_conf_merge_off_value(conf, prev, default)                        \
    if (conf == NGX_CONF_UNSET) {                                            \
        conf = (prev == NGX_CONF_UNSET) ? default : prev;                    \
    }

#define ngx_conf_merge_str_value(conf, prev, default)                        \
    if (conf.data == NULL) {                                                 \
        if (prev.data) {                                                     \
            conf.len = prev.len;                                             \
            conf.data = prev.data;                                           \
        } else {                                                             \
            conf.len = sizeof(default) - 1;                                  \
            conf.data = (u_char *) default;                                  \
        }                                                                    \
    }

#define ngx_conf_merge_bufs_value(conf, prev, default_num, default_size)     \
    if (conf.num == 0) {                                                     \
        if (prev.num) {                                                      \
            conf.num = prev.num;                                             \
            conf.size = prev.size;                                           \
        } else {                                                             \
            conf.num = default_num;                                          \
            conf.size = default_size;                                        \
        }                                                                    \
    }

#define ngx_conf_merge_bitmask_value(conf, prev, default)                    \
    if (conf == 0) {                                                         \
        conf = (prev == 0) ? default : prev;                                 \
    }


char *ngx_conf_param(ngx_conf_t *cf);
char *ngx_conf_parse(ngx_conf_t *cf, ngx_str_t *filename);
char *ngx_conf_include(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);


ngx_int_t ngx_conf_full_name(ngx_cycle_t *cycle, ngx_str_t *name,
    ngx_uint_t conf_prefix);
ngx_open_file_t *ngx_conf_open_file(ngx_cycle_t *cycle, ngx_str_t *name);
void ngx_cdecl ngx_conf_log_error(ngx_uint_t level, ngx_conf_t *cf,
    ngx_err_t err, const char *fmt, ...);


char *ngx_conf_set_flag_slot(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
char *ngx_conf_set_str_slot(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
char *ngx_conf_set_str_array_slot(ngx_conf_t *cf, ngx_command_t *cmd,
    void *conf);
char *ngx_conf_set_keyval_slot(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
char *ngx_conf_set_num_slot(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
char *ngx_conf_set_size_slot(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
char *ngx_conf_set_off_slot(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
char *ngx_conf_set_msec_slot(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
char *ngx_conf_set_sec_slot(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
char *ngx_conf_set_bufs_slot(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
char *ngx_conf_set_enum_slot(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
char *ngx_conf_set_bitmask_slot(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);


extern ngx_uint_t     ngx_max_module;
extern ngx_module_t  *ngx_modules[];         /*  在编译脚本自动生成的ngx_modules.c文件中定义 */


#endif /* _NGX_CONF_FILE_H_INCLUDED_ */
