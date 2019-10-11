#include <stdio.h>
#include "cthread.h"

void *thread_f(void *arg)
{
    pid_t thread_id = getpid();
    pid_t pid = getpid();
    printf("pid %d, tid %d: new thread, arg = %d\n", (int)pid, (int)thread_id, *((int*)arg));
    return 0;
}

void *thread_d(void *arg)
{
    struct imp *answer = (struct imp *) arg;
    printf("thread started\n");
    return 0;
}

int main()
{
    cthread_t thread[5];
    for(int i = 0; i < 5; i++)
    {
        cthread_create(&thread[i], thread_d, NULL);
        cthread_detach(&thread[i]);
    }
    for(int i = 0; i < 5; i++)
    {
        if(!thread[i].is_finished)
        {
            i = 0;
            sched_yield();
        }
    }
    printf("detached thread finished\n");
    return 0;
}