/******************************************************************************

 @File Name : {PROJECT_DIR}/sync.h

 @Creation Date : 30-01-2012

 @Last Modified : Wed 01 Feb 2012 01:42:43 AM CST

 @Created By: Wenxuan Zhou

 @Purpose :

*******************************************************************************/
#ifndef _SYNC_H
#define _SYNC_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <pthread.h>
#include <stdint.h>
#include <time.h>
#include <errno.h>

  typedef struct WaitObject {
    int tag;
    int mem;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
  } WaitObject;

  typedef struct LockObject {
    pthread_mutex_t mutex;
  } LockObject;

  static inline LockObject* new_lock()
  {
    LockObject* l = (LockObject*) malloc(sizeof(WaitObject));
    pthread_mutex_init(&l->mutex, NULL);
    return l;
  }
  
  static inline void free_lock(LockObject* l)
  {
    pthread_mutex_destroy(&l->mutex);
    free(l);
  }
  
  static inline void syn_lock(LockObject* l)
  {
    pthread_mutex_lock(&l->mutex);
  }

  static inline int syn_trylock(LockObject* l)
  {
    if (pthread_mutex_trylock(&l->mutex) == EBUSY)
      return 0;
    return 1;
  }

  static inline void syn_unlock(LockObject* l)
  {
    pthread_mutex_unlock(&l->mutex);
  }

  static inline WaitObject* new_wait(int tag)
  {
    WaitObject* w = (WaitObject*) malloc(sizeof(WaitObject));
    w->tag = tag;
    w->mem = 0;
    pthread_mutex_init(&w->mutex, NULL);
    pthread_cond_init(&w->cond, NULL);
    return w;
  }

  static inline void free_wait(WaitObject* w)
  {
    pthread_cond_destroy(&w->cond);
    pthread_mutex_destroy(&w->mutex);
    free(w);
  }

  static inline void wait_cond(WaitObject* wobj)
  {
    pthread_mutex_lock(&wobj->mutex);
    while (wobj->mem != 1)
      pthread_cond_wait(&wobj->cond, &wobj->mutex);
    wobj->mem = 0;
    pthread_mutex_unlock(&wobj->mutex);
  }

#include <stdio.h>
#include <errno.h>
#include <string.h>
  static inline void wait_cond_to(WaitObject* wobj, uint64_t nsec)
  {
    struct timespec t;

    clock_gettime(CLOCK_REALTIME, &t);
    uint64_t tmp = nsec + t.tv_nsec;
    t.tv_nsec = tmp % 1000000000;
    t.tv_sec += tmp / 1000000000;

    pthread_mutex_lock(&wobj->mutex);
    while (wobj->mem != 1) {
      int ret = pthread_cond_timedwait(&wobj->cond, &wobj->mutex, &t);
      if (ret == ETIMEDOUT)
        break;
      else if (ret != 0) {
        DEBUG("error in timed wait: %s\n", strerror(ret));
        break;
      }
    }
    wobj->mem = 0;
    pthread_mutex_unlock(&wobj->mutex);
  }

  static inline void wake_cond(WaitObject* wobj)
  {
    pthread_mutex_lock(&wobj->mutex);
    wobj->mem = 1;
    pthread_cond_signal(&wobj->cond);
    pthread_mutex_unlock(&wobj->mutex);
  }

  static inline void wake_cond_all(WaitObject* wobj)
  {
    pthread_mutex_lock(&wobj->mutex);
    wobj->mem = 1;
    pthread_cond_broadcast(&wobj->cond);
    pthread_mutex_unlock(&wobj->mutex);
  }

#ifdef __cplusplus
}
#endif



#endif
