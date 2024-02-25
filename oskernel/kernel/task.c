#include "linux/task.h"
#include "linux/printk.h"
#include "linux/mm.h"
#include "linux/memory.h"
#include "lib/string.h"
#include "linux/delay.h"
#include "linux/gdt.h"



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
extern int jiffy;
extern int cpu_tickes;

extern void sched_task(void);
extern void task_deal_sleep(void);
extern void mov_to_user_mode(void);

task_t *g_tasks[TASK_MAX_NUMS];

static int get_free_task_index()
{
    for(int i = 0; i < TASK_MAX_NUMS; ++i) {
        if(!g_tasks[i])
            return i;
    }
    return -1;
}

/* 
    编写gdb测试脚本验证调度算法的正确性：
 */
task_t *get_next_ready_task()
{
    task_t *highPrioTask = NULL;
    bool hasWaitTask = false; //有 TASK_WAITING 状态的任务

    /* 当前情况下，有n中任务类型：idle任务、ready任务、waiting任务（时间片执行完了）、睡眠任务 */
    for(int i = 1; i < TASK_MAX_NUMS; ++i) {
        if(!g_tasks[i])
            continue;

        if(g_tasks[i]->state == TASK_WAITING) {
            hasWaitTask = true;
            continue;
        }

        if(g_tasks[i]->state == TASK_READY) { //找到ready的任务
            if(!highPrioTask) { //第一个ready任务
                highPrioTask = g_tasks[i];
                continue;
            }
            
            if(g_tasks[i]->priority > highPrioTask->priority) {   //比上个任务的优先级高
                highPrioTask = g_tasks[i];
            }
        }
    }

    if(!highPrioTask ) {
        if(hasWaitTask) { //没有ready任务，且含有waiting任务，此时需要重置waiting任务的时间片
            for(int i = 1; i < TASK_MAX_NUMS; ++i) {
                if(!g_tasks[i] || g_tasks[i]->state != TASK_WAITING)
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
        } else { //只有idle任务，没有waiting和ready任务，可能有sleep任务。
            highPrioTask = g_tasks[0];
        }
    }

    return highPrioTask;
}

task_t *ktask_create(const char *name, task_fn func, size_t stackSize, size_t priority)
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

    newTask->stack = kmalloc(PAGE_SIZE); //分配内核任务栈
    if(!newTask->stack) {
        printk("tasks create fail, no memory.\n");
        kfree_s(newTask, sizeof(*newTask));
        return NULL;
    }
    
    newTask->user_stack = kmalloc(stackSize); //分配用户任务栈
    if(!newTask->user_stack) {
        printk("tasks create fail, no memory.\n");
        kfree_s(newTask->stack, PAGE_SIZE);
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
    newTask->context.esp = ((u32)newTask->stack + PAGE_SIZE - 1); //满减栈
    newTask->context.eip = (u32)func;
    newTask->context.cs = KERNEL_CODE_SECTOR;
    newTask->context.ds = KERNEL_DATA_SECTOR;
    newTask->context.es = KERNEL_DATA_SECTOR;
    newTask->context.fs = KERNEL_DATA_SECTOR;
    newTask->context.gs = KERNEL_DATA_SECTOR;
    newTask->context.ss = KERNEL_DATA_SECTOR;

    newTask->esp0 = newTask->context.esp;
    newTask->ebp0 = newTask->esp0;
    newTask->esp3 = ((u32)newTask->user_stack + stackSize - 1);
    newTask->ebp3 = newTask->esp3;

    newTask->counter = priority;
    newTask->priority = priority; //时间片和优先级值相等

    newTask->state = TASK_READY;
    g_tasks[pid] = newTask;

    return newTask;
}

task_t *utask_create(const char *name, task_fn func, size_t stackSize, size_t priority)
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

    newTask->stack = kmalloc(PAGE_SIZE); //分配内核任务栈
    if(!newTask->stack) {
        printk("tasks create fail, no memory.\n");
        kfree_s(newTask, sizeof(*newTask));
        return NULL;
    }
    
    newTask->user_stack = kmalloc(stackSize); //分配用户任务栈
    if(!newTask->user_stack) {
        printk("tasks create fail, no memory.\n");
        kfree_s(newTask->stack, PAGE_SIZE);
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
    newTask->context.esp = ((u32)newTask->stack + PAGE_SIZE - 1); //满减栈
    newTask->context.eip = (u32)func;
    newTask->context.cs = USER_CODE_SECTOR;
    newTask->context.ds = USER_DATA_SECTOR;
    newTask->context.es = USER_DATA_SECTOR;
    newTask->context.fs = USER_DATA_SECTOR;
    newTask->context.gs = USER_DATA_SECTOR;
    newTask->context.ss = USER_DATA_SECTOR;

    newTask->esp0 = newTask->context.esp;
    newTask->ebp0 = newTask->esp0;
    newTask->esp3 = ((u32)newTask->user_stack + stackSize - 1);
    newTask->ebp3 = newTask->esp3;

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
        // printk("*******idle task, cycle times %d ...\n", cout);
        cout++;
    }
}

static void init_func()
{
    mov_to_user_mode();
    // static unsigned int cout = 0;
    // printk("enter kernel task....\n");
    // while(1) {
    //     task_sleep(3000);
    //     printk("-------kernel task, cycle times %d ...\n", cout);
    //     cout++;
    // }
}

static void test_func()
{
    static unsigned int cout = 0;
    printk("enter user task....\n");
    while(1) {
        // task_sleep(1000);
        // delay_ms(500);
        printk("#######user task, cycle times %d ...\n", cout);
        cout++;
    }
}

static void create_idle()
{
    task_t *task;
    task = ktask_create("idle", idle_func, 2048, 1);
    set_runPL(task, 0);

    task = ktask_create("init", init_func, 2048, 3);
    set_runPL(task, 3);

    task = ktask_create("user task", test_func, 2048, 2);
    set_runPL(task, 0);
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
    // printk("Current task %s \n", task->name);
    // printk("couter %d.\n ", task->counter);
    task_wakeup(); //没执行一次时钟中断检查任务唤醒情况

    if(!task->counter) { //任务时间片已执行完，需要调度下一个任务
        task->state = TASK_WAITING;
        return 1;
    }
    task->counter--;
    return 0;
}

void task_sleep(int ms)
{
    task_t *sleepTask = CURRENT;
    if(!CURRENT) {
        printk("error! call in task...\n");
        return;
    }

    CLOSE_INTERRUPT();
    sleepTask->counter = cpu_tickes + ms/jiffy; //这里有个循环计数问题需要解决。
    sleepTask->state = TASK_SLEEPING;
    task_deal_sleep();
}

void task_wakeup()
{
    for(int i = 1; i < TASK_MAX_NUMS; ++i) {
        if(!g_tasks[i])
            continue;
        if(g_tasks[i]->state == TASK_SLEEPING) {
            if(cpu_tickes >= g_tasks[i]->counter) { //延时时间到了
                g_tasks[i]->state = TASK_READY;
                g_tasks[i]->counter = g_tasks[i]->priority;
            }
        }
    }
}

u32 get_esp3(const task_t *task)
{
    return task->esp3;
}

void set_runPL(task_t *task, u32 PL)
{
    task->runPL = PL;
}

u32 get_runPL(const task_t *task)
{
    return task->runPL;
}

