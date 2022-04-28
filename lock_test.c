//
// Created by luo2 on 2022/4/26.
//

#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <math.h>

pthread_mutex_t thread_mutex[1], mid_lock, data_lock, data_thread_lock;

struct param {
    //long threadnum;
    int n;
};

struct param params[6];
struct param params_thread[5];


void cal() {
    for (int t = 0; t < 5; t++) {
        printf("cal print, Result = %d \n", params_thread[t].n);
    }
}

void *thread_func_cal(void *param) {
    while (1) {
        pthread_mutex_lock(&thread_mutex[1]);
        pthread_mutex_lock(&data_thread_lock);
        printf("waiting for data...\n");
        cal();
        pthread_mutex_unlock(&data_thread_lock);
        pthread_mutex_unlock(&mid_lock);

    }
}

void *thread_func_mid(void *param) {
    while (1) {
        pthread_mutex_lock(&mid_lock);
        pthread_mutex_lock(&data_lock);
        pthread_mutex_lock(&data_thread_lock);
        params_thread[0] = params[0];
        params_thread[1] = params[1];
        params_thread[2] = params[2];
        params_thread[3] = params[3];
        params_thread[4] = params[4];
        pthread_mutex_unlock(&data_thread_lock);
        pthread_mutex_unlock(&data_lock);
        pthread_mutex_unlock(&thread_mutex[1]);
    }
}

int main(int argc, char *argv[]) {
    pthread_t my_thread;
    if (pthread_create(&my_thread, NULL, thread_func_cal, NULL) != 0)
        perror("pthread_create");
    if (pthread_create(&my_thread, NULL, thread_func_mid, NULL) != 0)
        perror("pthread_create");

    // sleep 5s cal 100 lines
    sleep(5);
    pthread_mutex_lock(&data_lock);
    //    struct param params = {{1},{2},{3},{4},{5}};
    struct param p = {1};
    struct param p2 = {2};
    struct param p3 = {3};
    struct param p4 = {4};
    struct param p5 = {5};
    struct param p6 = {6};
    params[0] = p;
    params[1] = p2;
    params[2] = p3;
    params[3] = p4;
    params[4] = p5;
    params[5] = p6;
    pthread_mutex_unlock(&data_lock);
    pthread_mutex_unlock(&mid_lock);


    sleep(5);
    pthread_mutex_lock(&data_lock);
    //    struct param params = {{1},{2},{3},{4},{5}};
    struct param bp = {25};
    struct param bp2 = {789};
    struct param bp3 = {23};
    struct param bp4 = {76};
    struct param bp5 = {15};
    struct param bp6 = {156};
    params[0] = bp;
    params[1] = bp2;
    params[2] = bp3;
    params[3] = bp4;
    params[4] = bp5;
    params[5] = bp6;
    pthread_mutex_unlock(&data_lock);
    pthread_mutex_unlock(&mid_lock);
    sleep(5);
}
