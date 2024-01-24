#include "linux/task.h"
#include "linux/printk.h"
#include "linux/mm.h"
#include "linux/memory.h"
#include "lib/string.h"
#include "linux/delay.h"



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
extern task_t *CURRENT;
extern void sched_task(void);

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
    const task_t *highPrioTask = NULL;
    bool notJustIdle = false; //除了idle还有其它任务

    for(int i = 1; i < TASK_MAX_NUMS; ++i) {
        if(!g_tasks[i])
            continue;
        notJustIdle = true;
        if(g_tasks[i]->state == TASK_READY) { //找到ready的任务
            if(!highPrioTask) { //第一个ready任务
                if(g_tasks[i]->counter)    //时间片为0说明该任务已经执行完自己的时间片了。
                    highPrioTask = g_tasks[i];
                continue;
            }
            
            if((g_tasks[i]->priority > highPrioTask->priority) && g_tasks[i]->counter) {   //比上个任务的优先级高，并且时间片不为0
                highPrioTask = g_tasks[i];
            }
        }
    }

    if(!highPrioTask ) {
        if(notJustIdle) { //除了idle还有其它任务，但是这些任务的时间片都执行完了，重置所有任务的时间片，并调度优先级最高的任务。
            for(int i = 1; i < TASK_MAX_NUMS; ++i) {
                if(!g_tasks[i])
                    continue;
                g_tasks[i]->counter = g_tasks[i]->priority; //重置时间片
                if(!highPrioTask) {
                    highPrioTask = g_tasks[i];
                    continue;
                }

                if(g_tasks[i]->priority > highPrioTask->priority) {   //比上个任务的优先级高
                    highPrioTask = g_tasks[i];
                }
            }
        } else { //只有idle任务
            highPrioTask = g_tasks[0];
        }
    }

    return highPrioTask;
}

task_t *task_create(const char *name, task_fn func, size_t stackSize, size_t priority)
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

    newTask->counter = priority;
    newTask->priority = priority; //时间片和优先级值相等

    newTask->state = TASK_READY;
    g_tasks[pid] = newTask;

    return newTask;
}

extern void construct_test_scene();
static void idle_func()
{
    static unsigned int cout = 0;
    printk("enter idle task....\n");
    while(1) {
        delay_ms(1000);
        printk("*******idle task, cycle times %d ...\n", cout);
        cout++;
    }
}

static void kernel_func()
{
    static unsigned int cout = 2;
    printk("enter kernel task....\n");
    while(cout) {
        delay_ms(1000);
        printk("-------kernel task, cycle times %d ...\n", cout);
        cout--;
    }
}

static void user_func()
{
    static unsigned int cout = 3;
    printk("enter user task....\n");
    while(cout) {
        delay_ms(1000);
        printk("#######user task, cycle times %d ...\n", cout);
        cout--;
    }
}

static void create_idle()
{
    task_create("idle", idle_func, 2048, 1);
    task_create("kernel task", kernel_func, 2048, 3);
    task_create("user task", user_func, 2048, 2);
}

void init_task()
{
    create_idle();
}

int get_first_sched_flag(task_t *task)
{
    if(task->fist_sched == 0)
        return 0;
    task->fist_sched = 0;
    return 1;
}

void set_task_ready(task_t *task)
{
    task->state = TASK_READY;
}

void task_exit(task_t *task)
{
    printk("<<<<<< exit task: %s.\n", task->name);
    g_tasks[task->pid] = NULL;

    kfree_s(task->stack, task->stackSize);
    kfree_s(task, sizeof(task_t));
    //清理当前任务后，我们需要把CURENT切换到下一个待调度任务，并主动调度
    CURRENT = get_next_ready_task();
    CURRENT->state = TASK_RUNNING;
    sched_task();
}

int sub_task_counter(task_t *task)
{
    // printk("Current task %s ", task->name);
    // printk("couter %d.\n ", task->counter);
    if(!task->counter) //任务时间片已执行完，需要调度下一个任务
        return 1;
    task->counter--;
    return 0;
}
