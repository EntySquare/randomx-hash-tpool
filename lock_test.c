//
// Created by luo2 on 2022/4/26.
//

#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

void cal(char *argv[])
{
   // 打印 argv 全部内容
}

void *thread_func_cal(void *param)
{
    while(1)
    {

        cal();
    }
}

void *thread_func_mid(void *param)
{
    while(1)
    {


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

    // sleep 5s cal 200 lines

    // sleep 5s cal 300 lines

}
