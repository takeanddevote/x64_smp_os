#include <iostream>
#include "thread.h"
#include "logger.h"
#include <cstring>
#include <mutex>
#include <pthread.h>
#include <string>
#include <utility>


std::mutex g_mutex;
static std::map<int, ns_thread_t *> g_all_threads_id;
static std::map<std::string, ns_thread_t *> g_all_threads_name;

static void thread_insert(ns_thread_t *thread)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    g_all_threads_id.insert(std::make_pair(thread->thread_id, thread));
    g_all_threads_name.insert(std::make_pair(std::string(thread->name), thread));
}

static void thread_remove(ns_thread_t *thread)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    g_all_threads_id.erase(thread->thread_id);
    g_all_threads_name.erase(thread->name);
}


ns_thread_t *get_thread_by_name(const char *name)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    return g_all_threads_name[std::string(name)];
}

ns_thread_t *get_thread_by_id(int id)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    return g_all_threads_id[id];
}




struct thread_priv {
    ns_thread_t *thread;
    void *(*start_routine)(void *);
    void *priv;
};

static void *pre_start_routine(void *priv)
{
    ns_thread_t *thread = (ns_thread_t *)priv;

    thread->thread_id = pthread_self();

    thread_insert(thread);

    return thread->start_routine(thread->priv);
}

int nst_create(ns_thread_t *thread, void *(*start_routine)(void *), const char *name, void *priv)
{
    thread->cond = PTHREAD_COND_INITIALIZER;
    thread->mutex = PTHREAD_MUTEX_INITIALIZER;
    thread->priv = priv;
    thread->start_routine = start_routine;
    thread->be_posted = false;
    strcpy(thread->name, name);
    return pthread_create(&thread->thread, NULL, pre_start_routine, thread);
}

int nst_wait(ns_thread_t *thread)
{
    return 0;
    pthread_mutex_lock(&thread->mutex);
    while(1) {
        pthread_cond_wait(&thread->cond, &thread->mutex);
    }
    pthread_mutex_unlock(&thread->mutex);
}

int nst_wait_by_name(const char *wait)
{
    ns_thread_t *curthread = get_thread_by_id(pthread_self()); /* 当前线程描述符 */
    ns_thread_t *waitthread = get_thread_by_name(wait);     /* 等待线程描述符 */

    curthread->be_posted = false;
    pthread_mutex_lock(&waitthread->mutex);
    waitthread->wait_this.insert(std::make_pair(std::string(curthread->name), curthread));
    while(curthread->be_posted == false) {
        pthread_cond_wait(&waitthread->cond, &waitthread->mutex);
    }
    pthread_mutex_unlock(&waitthread->mutex);
    curthread->be_posted = false;
    return 0;
}

int nst_post(ns_thread_t *wait_this)
{
    return 0;
    pthread_mutex_lock(&wait_this->mutex);
    pthread_cond_signal(&wait_this->cond);
    pthread_mutex_unlock(&wait_this->mutex);
}

int nst_post_by_name(const char *name)
{
    ns_thread_t *curthread = get_thread_by_id(pthread_self());  /* 当前线程描述符 */
    ns_thread_t *postthread = NULL;             /* 唤醒线程描述符 */

    pthread_mutex_lock(&curthread->mutex);

    /* 检查线程是否在等待列表中。存在则广播 */
    if(curthread->wait_this.find(std::string(name)) != curthread->wait_this.end()) {
        debugsit
        postthread = curthread->wait_this[std::string(name)];
        curthread->wait_this.erase(std::string(name));      /* 从队列中移除该线程 */
        postthread->be_posted = true;
        pthread_cond_broadcast(&curthread->cond);
    }
    pthread_mutex_unlock(&curthread->mutex);
    return 0;
}

int nst_destroy(ns_thread_t *thread)
{
    pthread_join(thread->thread, NULL);

    thread_remove(thread);
    pthread_mutex_destroy(&thread->mutex);
    pthread_cond_destroy(&thread->cond);
}