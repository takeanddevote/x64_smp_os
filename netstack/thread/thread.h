#ifndef __THREAD_H__
#define __THREAD_H__
#include <iostream>
#include <list>
#include <pthread.h>

// class ns_thread {
// public:
//     ns_thread();
//     ~ns_thread();
//     void nst_run(void *(*start_routine)(void *), void *priv);
//     void nst_wait(ns_thread &other);
//     void nst_wakeup_one(void);
//     void nst_wakeup_all(void);

//     pthread_t thread;
//     void *priv;
// private:
//     pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
//     pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
//     std::list<pthread_t *> wait_threads; /* 等待被此线程唤醒的线程 */
// } ;

typedef struct {
    char name[128];
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    pthread_t thread;
    void *priv;
} ns_thread_t;


int nst_create(ns_thread_t *thread, void *(*start_routine) (void *), void *priv);
int nst_wait(ns_thread_t *thread);
int nst_wait_by_name(const char *name);
int nst_post(ns_thread_t *wait_this);
int nst_post_by_name(const char *name);
int nst_destroy(ns_thread_t *thread);

#endif /* __THREAD_H__ */
