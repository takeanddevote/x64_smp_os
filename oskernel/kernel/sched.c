#include "linux/task.h"

extern task_t *get_next_ready_task();
extern void sched_task(void);

task_t *CURRENT = NULL;

/* 
梳理一下任务切换的过程：
1、首先创建一个idle任务描述符，并且手动构造一个现场：
        -返回地址eip、：即任务函数的地址，这样第一次任务调度就能从头执行任务函数。
        -r0栈esp0：即任务自己的栈，指向新分配的内存末尾地址（满减栈）。
    这个现场是第一次调度该任务时恢复的，后续的现场在时钟中断里保存，这样就能循环调度起来了。然后注册进任务链表中，然后进入kernel_main的死循环中。
2、时钟中断到来，从死循环跳转到时钟ISR中，并压栈eflags、cs、eip（此时是内核栈）。
3、时钟ISR检查是否第一次进入任务切换（CURRET==0？），如果是，则不需要保存现场（内核现场），直接执行调度sched即可
4、sched中取出要调度任务描述符，并赋值给CURRET，然后调用汇编函数switch_task执行任务切换。
5、switch_task直接恢复现场即可。
 */
void sched(void)
{
    task_t *next = get_next_ready_task();
    if(next) //如果没有ready任务，即不需要切换任务或者说切换回当前任务。
        CURRENT = next;

    CURRENT->state = TASK_RUNNING;
    sched_task();
}