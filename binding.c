/*
*gcc thread_test.c -lpthread
*/

#define _GNU_SOURCE
#include <stdio.h>
#include <math.h>
#include <pthread.h>
#include <unistd.h>
#include <sched.h>

void WasteTime(void)
{
    int abc = 1000;
    int temp = 0;

    while(abc--)
        temp = 10000*10000;

    sleep(1);
}

void *thread_func1(void *param)
{
    cpu_set_t cpu_set;
    while(1)
    {
        CPU_ZERO(&cpu_set);
        CPU_SET(1, &cpu_set);

        if (pthread_setaffinity_np(pthread_self(), sizeof(cpu_set),&cpu_set) < 0)
            perror("pthread_setaffinity_np");
        WasteTime();

        CPU_ZERO(&cpu_set);
        CPU_SET(2, &cpu_set);

        if (pthread_setaffinity_np(pthread_self(), sizeof(cpu_set),&cpu_set) < 0)
            perror("pthread_setaffinity_np");
        WasteTime();
    }
}

void *thread_func2(void *param)
{
    cpu_set_t cpu_set;
    while(1)
    {
        CPU_ZERO(&cpu_set);
        CPU_SET(3, &cpu_set);

        if (pthread_setaffinity_np(pthread_self(), sizeof(cpu_set),&cpu_set) < 0)
            perror("pthread_setaffinity_np");
        WasteTime();
    }
}

int main(int argc, char *argv[])
{
    pthread_t my_thread;
    cpu_set_t cpu_set;
    CPU_ZERO(&cpu_set);
    CPU_SET(0, &cpu_set);
    if (sched_setaffinity(0, sizeof(cpu_set), &cpu_set) < 0)
        perror("sched_setaffinity");

    if (pthread_create(&my_thread, NULL, thread_func1,NULL) != 0)
        perror("pthread_create");
    if (pthread_create(&my_thread, NULL, thread_func2,NULL) != 0)
        perror("pthread_create");

    while(1)
        WasteTime();

    pthread_exit(NULL);
}
著作权归作者所有。
商业转载请联系作者获得授权，非商业转载请注明出处。
作者：Ailson Jack
链接：http://www.only2fire.com/archives/55.html
来源：www.only2fire.com
