//
// Created by luo2 on 2022/4/25.
//
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <math.h>
#define NUM_THREADS 4


void *BusyWork(void *t) {
    int i;
    long tid = (long)t;
    double result=0.0;
    printf("Thread %ld starting...\n",tid);
    for (i=0; i<1000000; i++) {
        result = result + sin(i) * tan(i);
    }
    printf("Thread %ld done. Result = %e\n",tid, result);
    pthread_exit((void*) t);
}


int main (int argc, char *argv[]) {
    pthread_t thread[NUM_THREADS];
    pthread_attr_t attr;
    long t;
    void *status;
    /* Initialize and set thread detached attribute */
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    for (t = 0; t < NUM_THREADS; t++) {
        printf("Main: creating thread %ld\n", t);
        pthread_create(&thread[t], &attr, BusyWork, (void *) t);
    }
    /* Free attribute and wait for the other threads */
    pthread_attr_destroy(&attr);
    for (t = 0; t < NUM_THREADS; t++) {
        pthread_join(thread[t], &status);
        printf("Main: joined with thread %ld, status: %ld\n", t, (long) status);
    }
    printf("Main: program completed. Exiting.\n");
}