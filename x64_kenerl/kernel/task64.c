#include "linux/task64.h"
#include "linux/mm.h"
#include "linux/apic.h"
#include "linux/spinlock.h"
#include "logger.h"

#define TASK_MAX_NUMS 100

typedef struct {
    task_t *tasks[TASK_MAX_NUMS];
    spin_lock_handle task_lock;
} task_ctx_t;

static task_ctx_t g_task;

static int get_free_task_index()
{
    for(int i = 0; i < TASK_MAX_NUMS; ++i) {
        if(!g_task.tasks[i])
            return i;
    }
    return -1;
}

task_t *get_next_ready_task()
{
    task_t *highPrioTask = NULL;
    bool hasWaitTask = false; //有 TASK_WAITING 状态的任务

    spin_lock(&g_task.task_lock);
    /* 当前情况下，有n中任务类型：idle任务、ready任务、waiting任务（时间片执行完了）、睡眠任务 */
    for(int i = 0; i < TASK_MAX_NUMS; ++i) {
        if(!g_task.tasks[i])
            continue;

        if(g_task.tasks[i]->state == TASK_WAITING) {
            hasWaitTask = true;
            continue;
        }

        if(g_task.tasks[i]->state == TASK_READY) { //找到ready的任务
            if(!highPrioTask) { //第一个ready任务
                highPrioTask = g_task.tasks[i];
                continue;
            }
            
            if(g_task.tasks[i]->priority > highPrioTask->priority) {   //比上个任务的优先级高
                highPrioTask = g_task.tasks[i];
            }
        }
    }

    if(!highPrioTask ) {
        if(hasWaitTask) { //没有ready任务，且含有waiting任务，此时需要重置waiting任务的时间片
            for(int i = 1; i < TASK_MAX_NUMS; ++i) {
                if(!g_task.tasks[i] || g_task.tasks[i]->state != TASK_WAITING)
                    continue;
                g_task.tasks[i]->counter = g_task.tasks[i]->priority; //重置时间片
                if(!highPrioTask) {
                    highPrioTask = g_task.tasks[i];
                    continue;
                }

                if(g_task.tasks[i]->priority > highPrioTask->priority) {   //比上个任务的优先级高
                    highPrioTask = g_task.tasks[i];
                }
            }
        } else { //只有idle任务，没有waiting和ready任务，可能有sleep任务。
            if(g_task.tasks[0]->state == TASK_READY) {
                highPrioTask = g_task.tasks[0];
            } else {
                highPrioTask = NULL;
            }
        }
    }
    if(highPrioTask) {
        highPrioTask->state = TASK_RUNNING;
    }
    spin_unlock(&g_task.task_lock);
    return highPrioTask;
}

static void add_task(task_t *task)
{
    spin_lock(&g_task.task_lock);
    g_task.tasks[task->pid] = task;
    spin_unlock(&g_task.task_lock);
}

//创建内核态任务
task_t *task_create(const char *name, task_fun_t function, size_t stack_size, int priority)
{
    task_t *task = (task_t *)kzalloc(sizeof(task_t));
    if(!task) {
        err("create task fail.\n");
        return NULL;
    }

    task->stack_length = stack_size;
    task->stack = (char *)kzalloc(stack_size);
    if(!task->stack) {
        err("create task fail.\n");
        kfree_s(task, sizeof(task_t));
        return NULL;
    }
    // debug("create task name %s %p stack %p.\n", name, task, task->stack);
    task->pid = get_free_task_index();
    strcpy(task->name, name);
    task->state = TASK_INIT;
    task->counter = task->priority = priority;
    task->function = function;
    task->fisrt_sched = true;

    task->esp0 = task->stack + stack_size;
    task->cs = (3 << 3) | 0b000; // 内核任务
    task->ss = task->ds = (4 << 3) | 0b000;

    task->state = TASK_READY;
    add_task(task);
    return task;
}

static void *idle_thread(void *ptr)
{
    log("cpu %d enter idle_thread.\n", get_lapic_id());
    // while(1) {
    //     asm volatile("hlt;");
    // }
    return NULL;
}

static void *init_thread(void *ptr)
{
   log("cpu %d enter init_thread.\n", get_lapic_id());
    // while(1) {
    //     asm volatile("hlt;");
    // }
    return NULL;
}

int task_init()
{
    spin_lock_init(&g_task.task_lock);

    task_create("idle", idle_thread, PAGE_SIZE, 4);
    task_create("init1", init_thread, PAGE_SIZE, 3);
    task_create("init2", init_thread, PAGE_SIZE, 3);
    return 0;
}


bool get_first_sched_flag(task_t *task)
{
    return task->fisrt_sched;
}

void set_first_sched_flag(task_t *task, bool flag)
{
    task->fisrt_sched = flag;
}

void set_task_running(task_t *task)
{
    task->state = TASK_RUNNING;   
}

uint64_t get_task_esp0(task_t *task)
{
    return task->esp0;  
}

uint64_t get_task_cs(task_t *task)
{
    return task->cs;
}

uint64_t get_task_funtion(task_t *task)
{
    return (uint64_t)task->function;
}

uint64_t get_task_ss(task_t *task)
{
    return task->ss;
}

void task_clean(task_t *task)
{
    // debug("name %s %p stack %p.\n", task->name, task, task->stack);
    spin_lock(&g_task.task_lock);
    task->state = TASK_DIED;
    kfree_s(task->stack, task->stack_length);
    kfree_s(task, sizeof(task_t));

    g_task.tasks[task->pid] = NULL;
    spin_unlock(&g_task.task_lock);
}