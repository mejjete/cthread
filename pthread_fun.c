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
//#include "pthread.h"

volatile bool is_finished = false;
int counter = 0;

int thread_create_clone(int (*func)(void *), void *arg, void **stack);

void spin_lock(volatile bool *lock);
void spin_unlock(volatile bool *lock);

int futex_wait(int *futex, int val);
int futex_wake(int *futex);
void futex_lock(int *futex);
void futex_unlock(int *futex);

//int pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start)(void *), void *arg);

int thread_f(void *arg)
{
    pid_t thread_id = getpid();
    pid_t pid = getpid();
    printf("pid %d, tid %d: new thread, arg = %d\n", (int)pid, (int)thread_id, *((int*)arg));
    is_finished = true;
    return 0;
}

int thread_d(void *arg)
{
    printf("child thread\n");
    int i;
    for(i = 0; i < 100000; ++i)
        __sync_add_and_fetch(&counter, 1);
    is_finished = true;
    return 0;
}

int main()
{   
    printf("test\n");
    //pid_t thread_id = getpid();
    //pid_t pid = getpid();
    //printf("pid: %d, tid: %d new thread\n", (int)pid, (int)thread_id);
    void *stack;
    int arg = 100;
    int ret = thread_create_clone(thread_d, NULL, &stack);
    for(int i = 0; i < 100000; ++i)
        __sync_add_and_fetch(&counter, 1);
    while(!is_finished)
        sched_yield();
    printf("counter %d\n", counter);
    free(stack);
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

int thread_create_clone(int (*func)(void *), void *arg, void **stack)
{
    int stack_size = 65 * 1024;
    *stack = malloc(stack_size);
    void *stack_top = (char *) *stack + stack_size;
    int flags = CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND | CLONE_THREAD;
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

/*int pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start)(void *), void *arg)
{
    int stack_size = 65 * 1024;
    void *stack = malloc(stack_size);
    *stack_top = (char *) *stack + stack_size;
    int flags = CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND | CLONE_THREAD; 
    *thread = (pthread_t*)clone(start, stack_top, flags, attr);
    return *thread == 0 ? 0 : 1;
}*/