//x64 system
#ifndef _PTHREAD_H
#define _PTHREAD_H

#ifndef _BITS_PTHREADTYPES_H
#define _BITS_PTHREADTYPES_H 1 
#include <bits/wordsize.h>
#endif

#ifdef __x86_64__
#if WORDSIZE == 64
#  define __SIZEOF_PTHREAD_ATTR_T 56
#  define __SIZEOF_PTHREAD_MUTEX_T 40
#  define __SIZEOF_PTHREAD_MUTEXATTR_T 4
#  define __SIZEOF_PTHREAD_COND_T 48
#  define __SIZEOF_PTHREAD_CONDATTR_T 4
#  define __SIZEOF_PTHREAD_RWLOCK_T 56
#  define __SIZEOF_PTHREAD_RWLOCKATTR_T 8
#  define __SIZEOF_PTHREAD_BARRIER_T 32
#  define __SIZEOF_PTHREAD_BARRIERATTR_T 4
# else
#  define __SIZEOF_PTHREAD_ATTR_T 32
#  define __SIZEOF_PTHREAD_MUTEX_T 32
#  define __SIZEOF_PTHREAD_MUTEXATTR_T 4
#  define __SIZEOF_PTHREAD_COND_T 48
#  define __SIZEOF_PTHREAD_CONDATTR_T 4
#  define __SIZEOF_PTHREAD_RWLOCK_T 44
#  define __SIZEOF_PTHREAD_RWLOCKATTR_T 8
#  define __SIZEOF_PTHREAD_BARRIER_T 20
#  define __SIZEOF_PTHREAD_BARRIERATTR_T 4
# endif
#else
# define __SIZEOF_PTHREAD_ATTR_T 36
# define __SIZEOF_PTHREAD_MUTEX_T 24
# define __SIZEOF_PTHREAD_MUTEXATTR_T 4
# define __SIZEOF_PTHREAD_COND_T 48
# define __SIZEOF_PTHREAD_CONDATTR_T 4
# define __SIZEOF_PTHREAD_RWLOCK_T 32
# define __SIZEOF_PTHREAD_RWLOCKATTR_T 8
# define __SIZEOF_PTHREAD_BARRIER_T 20
# define __SIZEOF_PTHREAD_BARRIERATTR_T 4
#endif



typedef unsigned long int pthread_t;                 //thread inditification


typedef struct mutex
{

}pthread_mutex_t;                               //mutex


typedef struct p_mutex_attr
{

}pthread_mutexattr_t;                           //mutex attiribute


typedef struct p_cond
{

}pthread_cond_t;                                //condition variable


typedef struct p_cond_attr
{

}pthread_condattr_t;


typedef unsigned int pthread_key_t;             //condition variable argument object

typedef struct p_once
{

}pthread_once_t;                                //context control dynamical intializion


union pthread_attr_t
{
  char __size[__SIZEOF_PTHREAD_ATTR_T];
  long int __align;
};

typedef union pthread_attr_t pthread_attr_t;    //atributes thread


int pthread_create(pthread_t *thread, const pthread_attr_t *attr, 
                  void *(*start)(void *), void *arg);

int pthread_join(pthread_t thread_id, void **data);

#endif