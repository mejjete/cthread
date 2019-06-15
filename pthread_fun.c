//add thread destroy queue
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <syscall.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/utsname.h>
//#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <linux/sched.h>
#include <linux/futex.h>
#include <setjmp.h>
//#include "pthread.h"

typedef int (*cthread_f)(void*);

struct cthread_stack
{
    pid_t tid;
    void *stack;
    struct cthread_stack *next;
};

struct cthread
{
  int returned_code;
  cthread_f func;
  void *arg;
  struct cthread_stack *stack;
  bool lock;
  bool is_finished;
  bool is_detached;
  jmp_buf jmp;
};

volatile bool last_stack_lock = false;
static int COUNTER_FREE_STACK;
struct cthread_stack *stack_list = NULL;

static int thread_create_clone(int (*func)(void *), void *arg, void **stack /*pid_t *tid*/);

void spin_lock(volatile bool *lock);
void spin_unlock(volatile bool *lock);

int futex_wait(int *futex, int val);
int futex_wake(int *futex);
void futex_lock(int *futex);
void futex_unlock(int *futex);

int cthread_join(volatile struct cthread *thread);
void cthread_create(struct cthread *result, cthread_f func, void *arg);
int cthread_runner(void *arg);
void ctread_exit(struct cthread *thread, int retcode);
void cthread_destroy(struct cthread *thread);
void cthread_detach(struct cthread *thread);

//int pthread_create(pthread_t *thread, /* const pthread_attr_t *attr, void *(*start)(void *)*/ int (*func)(void *), void *arg);
//int pthread_join(pthread_t *thread_id);

int thread_f(void *arg)
{
    pid_t thread_id = getpid();
    pid_t pid = getpid();
    printf("pid %d, tid %d: new thread, arg = %d\n", (int)pid, (int)thread_id, *((int*)arg));
    return 0;
}

int thread_d(void *arg)
{
    printf("*****\nchild thread\n*****\n");
    return 0;
}

int main()
{   
    //pid_t thread_id = getpid();
    //pid_t pid = getpid();
    //printf("pid: %d, tid: %d new thread\n", (int)pid, (int)thread_id);
    struct cthread thread;
    int a = 18;
    cthread_create(&thread, thread_f, (void *) &a);
    while(!thread.is_finished)
        sched_yield();
    //printf("pid %d, tid %d: clone result = %d\n", (int)pid, (int)thread_id, ret);
    /*for(int i = 0; i < 100000; i++)
        __sync_add_and_fetch(&counter, 1);
    printf("counter: %d", counter);
    free(stack);
    return 0;*/
}

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

static int thread_create_clone(int (*func)(void *), void *arg, void **stack /*pid_t *tid*/)
{
    int stack_size = 65 * 1024;
    *stack = malloc(stack_size);
    void *stack_top = (char *) *stack + stack_size;
    int flags = CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND | CLONE_THREAD;
    printf("thread_create_clone\n");
    return clone(func, stack_top, flags, arg); 
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

/*int pthread_create(pthread_t *thread,  const pthread_attr_t *attr, void *(*start)(void *)int (*func)(void *), void *arg)
{
    if(thread == NULL)
        thread = (pthread_t *) malloc(sizeof(pthread_t));
    thread->returned_code = 0;
    thread->func = func;
    thread->arg = arg;
    thread->is_finished = false;
    thread->stack_size = 65 * 1024;
    return thread_create_clone(thread->func, thread->arg, thread->stack);
}

int pthread_join(pthread_t *thread_id)
{
    while(!thread_id->is_finished)
        sched_yield();
    free(thread_id->stack);
    return thread_id->returned_code;
}*/

int cthread_runner(void *arg)
{
    printf("cthread_runner\n");
    struct cthread *thread = (struct cthread *) arg;
    if(setjmp(thread->jmp) == 0)
    {
        arg = thread->arg;
        printf("cthread_runner: test: 160\narg: %d", *((int*)arg) );
        thread->returned_code = thread->func(thread->arg);
    }
    printf("cthread_runner: test: 162\n");
    if(thread->is_detached)
        cthread_destroy(thread);
    thread->is_finished = true;
    return 0;
}

void cthread_detach(struct cthread *thread)
{
    if(thread->is_finished)
        cthread_destroy(thread);
    thread->is_detached = true;
}

void cthread_create(struct cthread *result, cthread_f func, void *arg)
{
    printf("cthread_Create\n");
    result->stack = (struct cthread_stack *) malloc(sizeof(struct cthread_stack));
    result->returned_code = 0;
    result->func = func;
    result->arg = arg;
    result->is_finished = false;
    result->is_detached = false;
    printf("cthread_create: 189");
    thread_create_clone(cthread_runner, (void*) result, &result->stack->stack);
}

int cthread_join(volatile struct cthread *thread)
{
    while(!thread->is_finished)
        sched_yield();
    free(thread->stack);
    return thread->returned_code;
}

void ctread_exit(struct cthread *thread, int retcode)
{
    thread->returned_code = retcode;
    longjmp(thread->jmp, 1);
}

void cthread_destroy(struct cthread *thread)
{
    printf("thread is destroyed\n");
    spin_lock(&last_stack_lock);
    struct cthread_stack *iter = stack_list;
    while(iter != NULL)
    {
        if(iter->tid == 0)
            break;
        struct cthread_stack *next = iter->next;
        free(iter->stack);
        free(iter);
        iter = next;
        printf("stack is a freed\n");
    }
    thread->stack->next = iter;
    stack_list = thread->stack;
    spin_unlock(&last_stack_lock);
}