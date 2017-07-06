#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <stddef.h>
#include <sys/ioctl.h>
#include "wait.h"

void wait_init(wait_t *wait)
{
    if(wait){
        pthread_cond_init(&wait->wait, NULL);
        pthread_mutex_init(&wait->lock, NULL);
    }
}

void wait_complete(wait_t *wait)
{
    if(wait){
        pthread_mutex_lock(&wait->lock);
        pthread_cond_wait(&wait->wait, &wait->lock);
        pthread_mutex_unlock(&wait->lock);
    }
}

void wait_wakeup(wait_t *wait)
{
    if(wait){
        pthread_mutex_lock(&wait->lock);
        pthread_cond_signal(&wait->wait);
        pthread_mutex_unlock(&wait->lock);
    }
}

void wait_wakeup_all(wait_t *wait)
{
    if(wait){
        pthread_mutex_lock(&wait->lock);
        pthread_cond_broadcast(&wait->wait);
        pthread_mutex_unlock(&wait->lock);
    }
}

void wait_destroy(wait_t *wait)
{
    if(wait){
        pthread_cond_destroy(&wait->wait);
        pthread_mutex_destroy(&wait->lock);
    }
}
