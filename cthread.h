#ifndef _CTHREAD
#define _CTHREAD
/* for the Linux-like System */
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <syscall.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/utsname.h>
#include <string.h>
#include <unistd.h>
#include <linux/sched.h>
#include <linux/futex.h>
#include <setjmp.h>

typedef void *(*cthread_f)(void*);
typedef struct cthread
{
    void *returned_code;
    cthread_f func;
    void *arg;
    struct cthread_stack *stack;
    bool lock;
    bool is_finished;
    bool is_detached;
    jmp_buf jmp;
}cthread_t;

struct cthread_stack
{
    pid_t tid;
    void *stack;
    struct cthread_stack *next;
};

int thread_create_clone(void *(*func)(void *), void *arg, void **stack, pid_t *tid);

void spin_lock(volatile bool *lock);
void spin_unlock(volatile bool *lock);

int futex_wait(int *futex, int val);
int futex_wake(int *futex);
void futex_lock(int *futex);
void futex_unlock(int *futex);

void *cthread_join(volatile cthread_t *thread);
void cthread_create(cthread_t *result, cthread_f func, void *arg);
void *cthread_runner(void *arg);
void ctread_exit(cthread_t *thread, int retcode);
void cthread_destroy(cthread_t *thread);
void cthread_detach(cthread_t *thread);
#endif