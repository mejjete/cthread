#include <stdio.h>
#include "cthread.h"

volatile bool last_stack_lost = false;
struct cthread_stack *stack_list = NULL;

int futex_wait(int *futex, int val)
{
    return syscall(SYS_futex, futex, FUTEX_WAIT, val, NULL, NULL, 0);
}

int futex_wake(int *futex)
{
    return syscall(SYS_futex, futex, FUTEX_WAKE, 1, NULL, NULL, 0);
}

void futex_lock(int *futex)
{
    while(__sync_val_compare_and_swap(futex, 0, 1) != 0)
        futex_wait(futex, 1);
}

void futex_unlock(int *futex)
{
    __sync_bool_compare_and_swap(futex, 1, 0);
    futex_wake(futex);
}

int thread_create_clone(void *(*func)(void *), void *arg, void **stack, pid_t *tid)
{
    int stack_size = 65 * 1024;
    *stack = malloc(stack_size);
    void *stack_top = (char *) *stack + stack_size;
    int flags = CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND | CLONE_THREAD |
                CLONE_CHILD_CLEARTID | CLONE_CHILD_SETTID;
    return clone(func, stack_top, flags, arg, NULL, NULL, tid);
}

void spin_lock(volatile bool *lock)
{
    while(!__sync_bool_compare_and_swap(lock, 0, 1))
        ;
}

void spin_unlock(volatile bool *lock)
{
    __sync_bool_compare_and_swap(lock, 1, 0);
}

void *cthread_runner(void *arg)
{
    cthread_t *thread = (cthread_t *) arg;
    if(setjmp(thread->jmp) == 0)
    {
      arg = thread->arg;
      thread->returned_code = thread->func(thread->arg);
    }
    spin_lock(&thread->lock);
    if(thread->is_detached)
        cthread_destroy(thread);
    thread->is_finished = true;
    spin_unlock(&thread->lock);
    return 0;
}

void cthread_create(cthread_t *result, cthread_f func, void *arg)
{
    result->returned_code = NULL;
    result->func = func;
    result->arg = arg;
    result->is_finished = false;
    result->is_detached = false;
    result->lock = false;
    result->stack = malloc(sizeof(*result->stack));
    result->stack->next = NULL;
    void **stack_p = &result->stack->stack;
    pid_t *tid_p = &result->stack->tid;
    thread_create_clone(cthread_runner, (void*) result, stack_p, tid_p);
}

void *cthread_join(volatile cthread_t *thread)
{
    while(!thread->is_finished)
        sched_yield();
    free(thread->stack);
    return thread->returned_code;
}

void ctread_exit(cthread_t *thread, int retcode)
{
    *((int *)thread->returned_code) = retcode;
    longjmp(thread->jmp, 1);
}

void cthread_detach(cthread_t *thread)
{
    spin_lock(&thread->lock);
    if(thread->is_finished)
        cthread_destroy(thread);
    thread->is_detached = true;
    spin_unlock(&thread->lock);
}

void cthread_destroy(cthread_t *thread)
{
    printf("thread is destroyed\n");
    spin_lock(&last_stack_lost);
    struct cthread_stack *iter = stack_list;
    while(iter != NULL)
    {
      if(iter->tid != 0)
        break;
      struct cthread_stack *next = iter->next;
      free(iter->stack);
      free(iter);
      iter = next;
      printf("a stack is freed\n");
    }
    thread->stack->next = iter;
    stack_list = thread->stack;
    spin_unlock(&last_stack_lost);
    free(thread->stack);
}
