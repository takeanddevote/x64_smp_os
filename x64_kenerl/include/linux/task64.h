#ifndef __TASK64_H__
#define __TASK64_H__
#include "linux/type.h"
#include "list.h"

typedef void* (*task_fun_t)(void*);

typedef enum task_state_t {
    TASK_INIT,     // 初始化
    TASK_RUNNING,  // 执行
    TASK_READY,    // 就绪
    TASK_BLOCKED,  // 阻塞
    TASK_SLEEPING, // 睡眠
    TASK_WAITING,  // 等待
    TASK_DIED,     // 死亡
} task_state_t;

typedef struct {
    int64   rflags;     // 8 * 0
    int64   rip;        // 8 * 1
    int64   rax;        // 8 * 2
    int64   rbx;        // 8 * 3
    int64   rcx;        // 8 * 4
    int64   rdx;        // 8 * 5
    int64   rdi;        // 8 * 6
    int64   rsi;        // 8 * 7
    int64   rbp;        // 8 * 8
    int64   rsp;        // 8 * 9
    int64   r8;         // 8 * 10
    int64   r9;         // 8 * 11
    int64   r10;        // 8 * 12
    int64   r11;        // 8 * 13
    int64   r12;        // 8 * 14
    int64   r13;        // 8 * 15
    int64   r14;        // 8 * 16
    int64   r15;        // 8 * 17
    int64   cs;         // 8 * 18
    int64   ss;         // 8 * 19
    int64   ds;         // 8 * 20
    int64   fs;         // 8 * 21
    int64   gs;         // 8 * 22
} __attribute__((packed)) task_context_t;

typedef struct {
    task_context_t context;
    char name[64];
    uint64_t pid;
    task_fun_t function;
    uint64_t esp0;
    uint64_t esp3;
    uint64_t cs;
    uint64_t ss;
    uint64_t ds;
    char *stack;
    uint64_t stack_length;
    char *user_stack;
    uint64_t user_stack_length;
    uint64_t counter;
    uint64_t priority;
    task_state_t state;
    bool fisrt_sched;
    struct list_head node;
} __attribute__((packed)) task_t;


int task_init();
task_t *ktask_create(const char *name, task_fun_t function, size_t stack_size, int priority);
task_t *utask_create(const char *name, task_fun_t function, size_t stack_size, int priority);
task_t *get_next_ready_task();
bool get_first_sched_flag(task_t *task);
void reset_first_sched_flag(task_t *task);
void set_task_running(task_t *task);
void set_task_waiting(task_t *task);
uint64_t get_task_esp0(task_t *task);
uint64_t get_task_esp3(task_t *task);
void set_task_esp0(task_t *task, uint64_t esp0);
void set_task_esp3(task_t *task, uint64_t esp3);
uint64_t get_task_cs(task_t *task);
uint64_t get_task_funtion(task_t *task);
void task_clean(task_t *task);
int64_t decrease_task_couter(task_t *task);

#endif /* __TASK64_H__ */
