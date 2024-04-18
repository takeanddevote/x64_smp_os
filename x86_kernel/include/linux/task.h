#ifndef __TASK_H__
#define __TASK_H__
#include "linux/type.h"

#define TASK_MAX_NUMS 4

typedef void (*task_fn)(void);

typedef enum {
    TASK_INIT,
    TASK_RUNNING,
    TASK_READY,
    TASK_BLOCKED,
    TASK_SLEEPING,
    TASK_WAITING,
    TASK_DIED,
} task_state_e;


typedef struct tss_t {
    u32 backlink; //0 前一个任务的链接，保存了前一个任状态段的段选择子
    u32 esp0;     //1 ring0 的栈顶地址
    u32 ss0;      //2 ring0 的栈段选择子
    u32 esp1;     //3 ring1 的栈顶地址
    u32 ss1;      //4 ring1 的栈段选择子
    u32 esp2;     //5 ring2 的栈顶地址
    u32 ss2;      //6 ring2 的栈段选择子
    u32 cr3;    //7
    u32 eip;    //8
    u32 flags;  //9
    u32 eax;    //10
    u32 ebx;    //11
    u32 ecx;    //12
    u32 edx;    //13
    u32 esp;    //14
    u32 ebp;    //15
    u32 esi;    //16
    u32 edi;    //17
    u32 es;     //18
    u32 cs;     //19
    u32 ss;     //20
    u32 ds;     //21
    u32 fs;     //22
    u32 gs;     //23
    u32 ldtr;          //24 局部描述符选择子
    u16 trace : 1;     //25 如果置位，任务切换时将引发一个调试异常
    u16 reversed : 15; //26 保留不用
    u16 iobase;        //27 I/O 位图基地址，16 位从 TSS 到 IO 权限位图的偏移
    u32 ssp;           //28 任务影子栈指针
} __attribute__((packed)) tss_t;




typedef struct {
    tss_t context; //当前任务的环境
    char name[16];
    int pid;
    task_state_e state;
    task_fn function;
    void *stack;
    void *user_stack;
    int stackSize;
    int fist_sched;
    size_t priority;    //任务优先级
    size_t counter;     //任务时间片，每调度一次递减直到0

    u32 esp0;
    u32 ebp0;
    u32 esp3;
    u32 ebp3;

    u32 runPL;
} task_t;

void init_task();
task_t *ktask_create(const char *name, task_fn func, size_t stackSize, size_t priority);
task_t *utask_create(const char *name, task_fn func, size_t stackSize, size_t priority);
int get_first_sched_flag(task_t *task);
void set_task_ready(task_t *task);

void task_sleep(int ms);
void task_wakeup();

void set_runPL(task_t *task, u32 PL);
u32 get_runPL(const task_t *task);

#endif