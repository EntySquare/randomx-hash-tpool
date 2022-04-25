//
// Created by luo2 on 2022/4/22.
//
#include "randomx.h"
#include "chunk_and_entropy.h"
#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

#define THREADS_COUNT 1
#define TIMES_PER_LIST 200
#define LIST_NUM 10

static int validate_hash(
        unsigned char hash[RANDOMX_HASH_SIZE],
        unsigned char difficulty[RANDOMX_HASH_SIZE])
{
    return memcmp(hash, difficulty, RANDOMX_HASH_SIZE);
}

struct param {
    //long threadnum;
    const char* key;
    int keySize;
    unsigned char* input;
    int inputSize;
    unsigned char* output;
};


//void hash_cal(randomx_vm *machine, const void *input, size_t inputSize, void *output)
void *hash_cal(void *paramsPtr)
{
    int times = TIMES_PER_LIST;
    int list_len = LIST_NUM;
    //long tid = ((struct param*)paramsPtr)->threadnum;
    printf("Thread starting...\n");

   randomx_flags flags_vm = RANDOMX_FLAG_DEFAULT;
    flags_vm |= RANDOMX_FLAG_HARD_AES;
    flags_vm |= RANDOMX_FLAG_JIT;
    randomx_flags flags_fast = RANDOMX_FLAG_DEFAULT;
    flags_fast |= RANDOMX_FLAG_JIT;

    printf("flags is %d\n", flags_vm);


    randomx_cache *myCache = randomx_alloc_cache(flags_fast);
    randomx_init_cache(myCache, ((struct param*)paramsPtr)->key, ((struct param*)paramsPtr)->keySize);
    randomx_dataset *myDataset = randomx_alloc_dataset(flags_fast);
    randomx_vm *myMachine = randomx_create_vm(flags_vm, myCache, myDataset);
//    printf("the flag is %d\n", flags);

    time_t start = time(NULL);
    time_t end;
    for (int k = 0; k < list_len * times; k++) {
        randomx_calculate_hash(myMachine, ((struct param*)paramsPtr)->input, ((struct param*)paramsPtr)->inputSize, ((struct param*)paramsPtr)->output);

        if ((k + 1) >= times && (k + 1) % times == 0) {
            end = time(NULL);
            printf("Thread: calc rate is %f h/s\n", times / difftime(end, start));
            start = time(NULL);
        }

        if ((k + 1) == list_len * times ){
            unsigned char* hash = ((struct param*)paramsPtr)->output;
            for (unsigned i = 0; i < RANDOMX_HASH_SIZE; ++i)
            { printf("%02x", hash[i] & 0xff); }
            printf("\n");
        }
    }

    randomx_destroy_vm(myMachine);

    pthread_exit( (void*) paramsPtr);

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

    struct param *parameters = (struct param *)malloc(sizeof(struct param));
    parameters->key = myKey;
    parameters->keySize = sizeof myKey;
    parameters->input = myInput;
    parameters->inputSize = sizeof myInput;
    parameters->output = hash;

//    pthread_t *thread_id = (pthread_t *)malloc(thread_count*sizeof(pthread_t));
    pthread_t thread_id[THREADS_COUNT];
    void *status;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    time_t start_total = time(NULL);
    for (long j = 0; j<THREADS_COUNT ; j++){
        pthread_create(&thread_id[j], &attr, hash_cal, (void *) parameters);
        printf("threads %ld is created\n", j+1);
    }

    pthread_attr_destroy(&attr);
    for (long k = 0; k<THREADS_COUNT ; k++){
        pthread_join(thread_id[k], &status);
        printf("threads %ld is done\n", k+1);
    }

    time_t end_total = time(NULL);
    printf("the parallel calc rate is %f h/s\n", TIMES_PER_LIST * LIST_NUM * THREADS_COUNT / difftime(end_total, start_total));

//    randomx_calculate_hash(myMachine, &myInput, sizeof myInput, hash);
//    for (unsigned i = 0; i < RANDOMX_HASH_SIZE; ++i)
//        printf("%02x", hash[i] & 0xff);

    if(validate_hash(hash, difficulty)>0)
    { printf("\nsolution found\n");}
    else
    { printf("\nsolution unfound\n");}

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
