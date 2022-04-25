//
// Created by luo2 on 2022/4/22.
//
#include "randomx.h"
//#include "chunk_and_entropy.h"
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

static int validate_hash(
        unsigned char hash[RANDOMX_HASH_SIZE],
        unsigned char difficulty[RANDOMX_HASH_SIZE])
{
    return memcmp(hash, difficulty, RANDOMX_HASH_SIZE);
}


struct param {
    randomx_vm *machine;
    unsigned char* input;
    int inputSize;
    unsigned char* output;
};

//void hash_cal(randomx_vm *machine, const void *input, size_t inputSize, void *output)
void *hash_cal(void *paramsPtr)
{
//    randomx_vm *machine1 = ((struct param*)params)->machine ;
//    const void *input1 = ((struct param*)params)->input;
//    size_t inputSize1 = ((struct param*)params)->inputSize;
//    void *output1 = ((struct param*)params)->output;
    time_t start = time(NULL);
    time_t end;
    int times = 100;

    for (int k = 0; k < 5 * times; k++) {
        randomx_calculate_hash(((struct param*)paramsPtr)->machine, ((struct param*)paramsPtr)->input, ((struct param*)paramsPtr)->inputSize, ((struct param*)paramsPtr)->output);

        if ((k + 1) >= times && (k + 1) % times == 0) {
            end = time(NULL);
            printf("calc rate is %f h/s\n", times / difftime(end, start));
            start = time(NULL);
        }
    }
    return 0 ;
}



int main()
{
    const char myKey[] = {255, 255,255, 254, 219, 155, 62, 29, 172, 210, 122, 149, 253, 169, 34, 24,
                          33, 152, 221, 38, 200, 234, 74, 60, 118, 235, 15, 159, 33, 237, 210, 127};
    const char h0[] = {236,97,53,71,37,0,200,215,7,52,32,198,108,183,90,4,140,41,110,170,32,109,7,56,229,47,186,12,150,63,52,232};
    const char prevh[] = {61, 222, 227, 151, 197, 175, 127, 142, 18, 210, 148, 122, 239, 9, 40, 9, 78, 47, 1, 208, 199, 19, 214, 225, 211, 93, 196, 144, 253, 232, 176, 145, 62, 172, 183, 229, 89, 16, 42, 96, 247, 44, 228, 20, 71, 71, 31, 85};
    const char timestampBinary[] = {0,0,0,0,0,0,0,0,98,93,21,92};
    const char chunk[] = RANDOMX_HASH_TPOOL_CHUNK_AND_ENTROPY;
    const char entropy[] = RANDOMX_HASH_TPOOL_CHUNK_AND_ENTROPY;
    unsigned char hash[RANDOMX_HASH_SIZE];
    unsigned char difficulty[] = {255,255,255,255,57,187,243,201,6,149,141,58,43,178,62,177,161,169,15,75,12,68,25,200,65,151,136,126,129,147,114,67};

    int jitEnabled=1, largePagesEnabled=1, hardwareAESEnabled=1;
    int len_h0 = sizeof(h0)/sizeof(char);
    int len_prevh = sizeof(prevh)/sizeof(char);
    int len_time = sizeof(timestampBinary)/sizeof(char);
    int len_chunk = sizeof(chunk)/sizeof(char);
    int len_entropy = sizeof(entropy)/sizeof(char);
    unsigned char myInput[len_h0 + len_prevh + len_time + len_chunk + len_entropy];

    for (int i = 0; i < len_h0 + len_prevh + len_time + len_chunk + len_entropy ; i++)
    {
        if (i< len_h0)
        {
            myInput[i] = h0[i];
        }
        else if (i< len_h0 + len_prevh && i >= len_h0)
        {
            myInput[i] = prevh[i - len_h0];
        }
        else if (i< len_h0 + len_prevh + len_time && i >= len_h0 + len_prevh)
        {
            myInput[i] = timestampBinary[i - len_h0 - len_prevh];
        }
        else if (i< len_h0 + len_prevh + len_time + len_chunk && i >= len_h0 + len_prevh + len_time)
        {
            myInput[i] = chunk[i - len_h0 - len_prevh - len_time];
        }
        else if (i< len_h0 + len_prevh + len_time + len_chunk + len_entropy && i >= len_h0 + len_prevh + len_time + len_chunk )
        {
            myInput[i] = entropy[i - len_h0 - len_prevh - len_time - len_chunk ];
        }

    }

    int lem = sizeof(myInput);
    printf("myinput data size is %d\n", lem);

    randomx_flags flags = randomx_get_flags();
    randomx_cache *myCache = randomx_alloc_cache(flags);
    randomx_init_cache(myCache, &myKey, sizeof myKey);
    randomx_vm *myMachine = randomx_create_vm(flags, myCache, randomx_alloc_dataset(flags));

    struct param *parameters;
    int input_len = sizeof myInput;
//    parameters->machine = myMachine;
    parameters->input = myInput;
    parameters->inputSize = input_len;
    parameters->output = hash;
    printf("structure ok");

    int thread_count = 5;
    pthread_t *thread_id = (pthread_t *)malloc(thread_count*sizeof(pthread_t));
    printf("threads creation starts");
    for (int j = 0; j<thread_count ; j++)
    {
        pthread_create(&thread_id[j], NULL, hash_cal, (void *) parameters);
        pthread_join(thread_id[j], NULL);
    }

    randomx_calculate_hash(myMachine, &myInput, sizeof myInput, hash);

    for (unsigned i = 0; i < RANDOMX_HASH_SIZE; ++i)
        printf("%02x", hash[i] & 0xff);

    if(validate_hash(hash, difficulty)>0)
    { printf("\nsolution found\n");}
    else
    { printf("\nsolution unfound\n");}

    randomx_destroy_vm(myMachine);
    randomx_release_cache(myCache);

    printf("\ntest done\n");

}




//    FILE *fp = NULL;
//    char test[]={1,2,3,4,5,6,7,8,9,0};
//    fp = fopen("bigdata.txt", "w+");
//    fputs(test, fp);
//    fwrite(chunk, sizeof(chunk), sizeof(chunk), fp);
//    fclose(fp);
//
//    FILE *fq = NULL;
//    char buff[10];
//    fq = fopen("bigdata.txt", "r");
//    fgets(buff, 256*1024, (FILE*)fq);
//    printf("%s\n", buff);
//    fclose(fq);


//  example
//#include<pthread.h>
//#include<stdio.h>
//void *workThreadEntry(void *args)
//{
//    char*str = (char*)args;
//    printf("threadId:%lu,argv:%s\n",pthread_self(),str);
//}
//
//int main(int argc, char *agrv[])
//{   pthread_t thread_id;
//    char*str = "hello world";
//    pthread_create(&thread_id,NULL,workThreadEntry,str);
//    printf("threadId=%lu\n",pthread_self());
//    pthread_join(thread_id,NULL);
//}
