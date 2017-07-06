#ifndef __WAIT_H__
#define __WAIT_H__
#include <pthread.h>

typedef struct{
    pthread_cond_t      wait;
    pthread_mutex_t     lock;
}wait_t;

void wait_init(wait_t *);
void wait_complete(wait_t *);
void wait_wakeup(wait_t *);
void wait_wakeup_all(wait_t *);
void wait_destroy(wait_t *);

#endif //__WAIT_H__

