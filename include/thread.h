#ifndef THREAD_H
#define THREAD_H

#include <pthread.h>

#define THREADPOOL_SZ 9	//Number of child threads a thread can have; one for each possible value of cell

extern pthread_once_t once_control;

void mutex_init(void);
int threadpool_spawn(pthread_t *threadpool, int *board);
int *threadpool_join(pthread_t *threadpool);
void threadpool_cancel(void *arg);

#endif
