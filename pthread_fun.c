#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
//#include <syscall.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/utsname.h>
//#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <linux/sched.h>

volatile bool is_finished = false;
volatile int counter = 0;

int thread_create_clone(int (*func)(void *), void *arg, void **stack);

int thread_f(void *arg)
{
    /*pid_t thread_id = getpid();
    pid_t pid = getpid();
    printf("pid %d, tid %d: new thread, arg = %d\n", (int)pid, (int)thread_id, *((int*)arg));
    is_finished = true;*/
    return 0;
}

int main()
{   
    pid_t thread_id = getpid();
    pid_t pid = getpid();
    printf("pid: %d, tid: %d new thread\n", (int)pid, (int)thread_id);
    void *stack;
    int arg = 100;
    int ret = thread_create_clone(thread_f, (void*) &arg, &stack);
    printf("pid %d, tid %d: clone result = %d\n", (int)pid, (int)thread_id, ret);
    while(!is_finished)
        sched_yield();
    free(stack);
    return 0;
}


int thread_create_clone(int (*func)(void *), void *arg, void **stack)
{
    int stack_size = 65 * 1024;
    *stack = malloc(stack_size);
    void *stack_top = (char *) *stack + stack_size;
    int flags = CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND | CLONE_THREAD;
    return clone(func, stack_top, flags, arg);
}