#include "linux/task.h"
#include "linux/printk.h"
#include "linux/mm.h"
#include "linux/memory.h"
#include "lib/string.h"



/* 
1、任务描述符：名字、状态、任务函数、任务id。所有的任务描述符放在一个全局链表中。
2、未创建任务之前，内核处于R0裸机状态，如何切换到多任务状态？任务切换有两种方式：任务主动调用sched、时钟中断切换。
3、如何实现主任务切换？
    任务切换的原理就是保护现场恢复现场，这个现场是寄存器+栈。当前任务要切到下一个任务时，先保存当前的所有寄存器（eip、esp、ebp、eflag等等）
    到任务描述符中，然后获取下一个任务的任务描述符，然后恢复该任务的现场。
4、特殊处理：如果是第一次进入调度，则不需要保存现场，因为当前现场是裸机内核的现场。
    如果线程是第一次调度，那就不需要恢复现场，而是构造一个初始现场给这个线程。初始化现场包括：任务栈、任务函数设为返回地址（当前中断结束后）、
    任务退出返回地址。
 */
task_t *g_tasks[TASK_MAX_NUMS];

static int get_free_task_index()
{
    for(int i = 0; i < TASK_MAX_NUMS; ++i) {
        if(!g_tasks[i])
            return i;
    }
    return -1;
}

task_t *get_next_ready_task()
{
    for(int i = 0; i < TASK_MAX_NUMS; ++i) {
        if(!g_tasks[i])
            continue;
        if(g_tasks[i]->state == TASK_READY)
            return g_tasks[i];

    }
    return NULL;
}

task_t *task_create(const char *name, task_fn func, size_t stackSize)
{
    int pid = get_free_task_index(); //获取空闲的任务id
    if(pid < 0) {
        printk("max tasks.\n");
        return NULL;
    }

    stackSize = stackSize > PAGE_SIZE ? PAGE_SIZE : stackSize;
    task_t *newTask = (task_t *)kmalloc(sizeof(task_t)); //分配任务描述符
    if(!newTask) {
        printk("tasks create fail, no memory.\n");
        return NULL;
    }

    newTask->stack = kmalloc(stackSize); //分配任务栈
    if(!newTask->stack) {
        printk("tasks create fail, no memory.\n");
        kfree_s(newTask, sizeof(*newTask));
        return NULL;
    }
    
    //初始化任务描述符
    newTask->stackSize = stackSize;
    // newTask->stack = (void *)((u32)newTask->stack +  stackSize - 1); //满减栈
    strcpy(newTask->name, name);
    newTask->function = func;//任务函数
    newTask->pid = pid;
    newTask->state = TASK_INIT;
    newTask->fist_sched = 1; //标记第一次调度

    //构造一个初始现场
    newTask->context.esp = ((u32)newTask->stack + stackSize - 1); //满减栈
    newTask->context.eip = (u32)func;

    newTask->state = TASK_READY;
    g_tasks[pid] = newTask;

    return newTask;
}

static void idle_func()
{
    printk("enter idle task....\n");
    while(1) {
    }
}

static void create_idle()
{
    task_create("idle", idle_func, 2048);
}

void init_task()
{
    create_idle();
}

int get_first_sched_flag(task_t *task)
{
    return task->fist_sched;
}

void construct_task_initial_scene(task_t *task)
{
    /* 初始化现场包括：任务栈、任务函数设为返回地址（当前中断结束后）、任务退出返回地址。*/
    /* 任务退出返回地址如何理解？实际就是构造一个类似call的栈环境，即call 任务函数，当任务函数结束时，
        就相当于正常的call调用函数的返回一样。 */

    task->fist_sched = 0;

}