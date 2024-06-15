#include "thread.h"
#include <pthread.h>

int nst_create(ns_thread_t *thread, void *(*start_routine) (void *), void *priv)
{
    thread->cond = PTHREAD_COND_INITIALIZER;
    thread->mutex = PTHREAD_MUTEX_INITIALIZER;
    thread->priv = priv;
    return pthread_create(&thread->thread, NULL, start_routine, priv);
}

int nst_wait(ns_thread_t *thread)
{
    pthread_mutex_lock(&thread->mutex);
    pthread_cond_wait(&thread->cond, &thread->mutex);
    pthread_mutex_unlock(&thread->mutex);
}

int nst_post(ns_thread_t *wait_this)
{
    pthread_mutex_lock(&wait_this->mutex);
    pthread_cond_signal(&wait_this->cond);
    pthread_mutex_unlock(&wait_this->mutex);
}

int nst_destroy(ns_thread_t *thread)
{
    pthread_join(thread->thread, NULL);

    pthread_mutex_destroy(&thread->mutex);
    pthread_cond_destroy(&thread->cond);
}




















// ns_thread::ns_thread()
// {

// }
// ns_thread::~ns_thread()
// {

// }
// void ns_thread::nst_run(void *(*start_routine)(void *), void *priv)
// {
//     pthread_create(&thread, NULL, start_routine, priv);
// }
// void ns_thread::nst_wait(ns_thread &other)
// {
//     wait_threads.push_back(&other.thread);
// }
// void ns_thread::nst_wakeup_one(void)
// {
//     std::list<pthread_t *>::iterator iter = wait_threads.begin();
//     if(iter == wait_threads.end())
//         return;

//     pthread_t *wait = *iter;

//     pthread_cond_signal(&cond);
//     wait_threads.erase(iter);   
// }
// void ns_thread::nst_wakeup_all(void)
// {

// }