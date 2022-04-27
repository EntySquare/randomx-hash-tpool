//
// Created by luo2 on 2022/4/26.
//

#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <math.h>

pthread_mutex_t mutex[1], main_lock;
long params = 100;

void cal(void *argv) {
    // 打印 argv 全部内容
    int times = (int)argv;
    double result = 0.0;
    for ( int t=0; t<times; t++) {
        for (int i = 0; i < 1000000; i++) {
            result = result + sin(i) * tan(i);
        }
       if (t+1 == times) printf("cal print, Result = %e\n", result);
    }
}

void *thread_func_cal(void *param)
{  int lo =0 ;
    while(1)
    {
        pthread_mutex_lock(&mutex[1]);
        if (lo == 0) { printf("waiting...\n"); }
        else {
            printf("params is %ld\n", params);
            cal((void*)params);
            printf("waiting...\n");
            pthread_mutex_unlock(&main_lock);
        }
        lo++;
    }
}

void *thread_func_mid(void *param)
{
    int l1 =0 ;
    while(1)
    {
        pthread_mutex_lock(&main_lock);
        sleep(1);
        pthread_mutex_unlock(&mutex[1]);
    }
}

int main(int argc, char *argv[])
{
    pthread_t my_thread;
    if (pthread_create(&my_thread, NULL, thread_func_cal,NULL) != 0)
        perror("pthread_create");
    if (pthread_create(&my_thread, NULL, thread_func_mid,NULL) != 0)
        perror("pthread_create");

    // sleep 5s cal 100 lines
    sleep(5);
    // sleep 5s cal 200 lines
    params = params + 100;
    sleep(5);
    // sleep 5s cal 300 lines
    params = params + 100;
    sleep(5);
}
