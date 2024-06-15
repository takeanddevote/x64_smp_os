#ifndef __THREAD_H__
#define __THREAD_H__
#include <iostream>
#include <list>
#include <pthread.h>
#include <string>
#include <map>

typedef struct ns_thread_t {
    char name[128];
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    pthread_t thread;

    void *priv;
    void *(*start_routine) (void *);

    int thread_id;
    bool be_posted;
    std::map<std::string, ns_thread_t*> wait_this;
} ns_thread_t;


int nst_create(ns_thread_t *thread, void *(*start_routine) (void *), const char *name, void *priv);
// int nst_wait(ns_thread_t *thread);
int nst_wait_by_name(const char *name); /* 当前线程睡眠，并加入 name 线程的唤醒队列 */
// int nst_post(ns_thread_t *wait_this);
int nst_post_by_name(const char *name); /* 唤醒等待当前线程的 name 线程 */
int nst_destroy(ns_thread_t *thread);

#endif /* __THREAD_H__ */
