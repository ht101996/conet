/*
 * =====================================================================================
 *
 *       Filename:  coroutine_impl.h
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  2014年05月06日 07时21分13秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (),
 *   Organization:
 *
 * =====================================================================================
 */
#ifndef __COROUTINE_IMPL_H_INCL__
#define __COROUTINE_IMPL_H_INCL__
#include "base/incl/list.h"
#include <ucontext.h>
#include "gc.h"
#include "timewheel.h"
#include "coroutine.h"
#include <map>
#include <sys/epoll.h>
#include "coctx.h"
#include "base/incl/fixed_mempool.h"
#include "gflags/gflags.h"
#include "base/incl/tls.h"

#ifdef USE_VALGRIND
#include <valgrind/valgrind.h>
#endif

DECLARE_int32(stack_size);

namespace conet
{

//struct epoll_ctx_t;
struct epoll_ctx_t
{
    int m_epoll_fd;
    int m_epoll_size;

    epoll_event *m_epoll_events;

    int wait_num;
};

struct fd_ctx_mgr_t;

struct coroutine_env_t
{
    list_head run_queue;
    coroutine_t * curr_co;
    coroutine_t * main;
    list_head tasks;
    uint64_t spec_key_seed;
    fixed_mempool_t default_stack_pool;
};


enum {
    CREATE=0,
    RUNNING=1,
    SUSPEND = 2,
    STOP=3,
    EXCEPTION_EXIT=-1,
};

struct coroutine_t
{
    ucontext_t ctx;
    //coctx_t ctx;
    
    void * stack;
    int stack_size;

    int state;

    struct {
        unsigned int is_main:1;
        unsigned int is_enable_sys_hook:1;
        unsigned int is_end_delete:1;
        unsigned int is_enable_pthread_hook:1;
        unsigned int is_enable_disk_io_hook:1;
        unsigned int is_page_stack:1;
    };

    int ret_val;
    void *yield_val;

    CO_MAIN_FUN *pfn;
    void *pfn_arg;

    char const *desc;
    gc_mgr_t * gc_mgr; // gc mem alloc manager


    list_head wait_to;

    list_head exit_notify_queue; // get notify on the co exit;

    uint64_t id;

#ifdef USE_VALGRIND
    int m_vid;
#endif

    std::map<void *, void*> *static_vars;
    std::map<uint64_t, void *> * spec;
    std::map<pthread_key_t, void *> * pthread_spec;

};

coroutine_env_t *alloc_coroutine_env();
void free_coroutine_env(void *arg);

extern __thread coroutine_env_t * g_coroutine_env;

inline
coroutine_env_t * get_coroutine_env()
{
    if (NULL == g_coroutine_env) {
        g_coroutine_env = alloc_coroutine_env();
        tls_onexit_add(g_coroutine_env, &free_coroutine_env);
    }
    return g_coroutine_env;
}

inline 
coroutine_t *get_curr_co_can_null()
{
    if ( NULL == g_coroutine_env) {
        return NULL;
    }
    return g_coroutine_env->curr_co;
}


}

#endif /* end of include guard */