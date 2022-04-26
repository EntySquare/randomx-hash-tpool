//
// Created by luo2 on 2022/4/25.
//

#include "randomx.h"
#include "./cpu.h"
#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <sched.h>

#define THREADS_COUNT 200
#define TIMES_PER_LIST 200
#define LIST_NUM 10

#define numWorkers 191


static int validate_hash(
        unsigned char hash[RANDOMX_HASH_SIZE],
        unsigned char difficulty[RANDOMX_HASH_SIZE])
{
    return memcmp(hash, difficulty, RANDOMX_HASH_SIZE);
}

struct param {
    randomx_flags flags;
    randomx_cache *cache;
    randomx_dataset *dataset;
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

    randomx_vm *myMachine = randomx_create_vm(((struct param*)paramsPtr)->flags, ((struct param*)paramsPtr)->cache, ((struct param*)paramsPtr)->dataset);

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
    FILE *fp;
    unsigned char wbuf[256*1024];
    u_int8_t c;
    fp = fopen("chunk.data","r");
    for(long i=0;i<256*1024;i++) {
        c = fgetc(fp);
        if (feof(fp)) {
            break;
        }
        wbuf[i]=c;
    }
    fclose(fp);

    unsigned char myKey[] = {255, 255,255, 254, 219, 155, 62, 29, 172, 210, 122, 149, 253, 169, 34, 24,
                                   33, 152, 221, 38, 200, 234, 74, 60, 118, 235, 15, 159, 33, 237, 210, 127};
    unsigned char h0[] = {236,97,53,71,37,0,200,215,7,52,32,198,108,183,90,4,140,41,110,170,32,109,7,56,229,47,186,12,150,63,52,232};
    unsigned char prevh[] = {61, 222, 227, 151, 197, 175, 127, 142, 18, 210, 148, 122, 239, 9, 40, 9, 78, 47, 1, 208, 199, 19, 214, 225, 211, 93, 196, 144, 253, 232, 176, 145, 62, 172, 183, 229, 89, 16, 42, 96, 247, 44, 228, 20, 71, 71, 31, 85};
    unsigned char timestampBinary[] = {0,0,0,0,0,0,0,0,98,93,21,92};
    unsigned char entropy[1024*256] ;
    unsigned char chunk[1024*256] ;
    memcpy(chunk,wbuf,sizeof(wbuf));
    memcpy(entropy,wbuf,sizeof(wbuf));
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

    randomx_flags flags_vm = RANDOMX_FLAG_FULL_MEM;
    flags_vm |= RANDOMX_FLAG_HARD_AES;
    flags_vm |= RANDOMX_FLAG_JIT;
    flags_vm |= RANDOMX_FLAG_LARGE_PAGES;
    flags_vm |= RANDOMX_FLAG_ARGON2_SSSE3;
    randomx_flags flags_fast = RANDOMX_FLAG_DEFAULT;
    flags_fast |= RANDOMX_FLAG_JIT;
//    printf("flags is %d\n", flags_vm);
    randomx_cache *myCache = randomx_alloc_cache(flags_fast);
    randomx_init_cache(myCache, &myKey, sizeof myKey);
    randomx_dataset *myDataset = randomx_alloc_dataset(flags_fast);

    unsigned long startItem = 0;
    unsigned long itemsPerThread = randomx_dataset_item_count() / numWorkers;
    unsigned long itemsRemainder = randomx_dataset_item_count() % numWorkers;
    unsigned long datasetInitStartItem;
    unsigned long datasetInitItemCount;
    for (int i = 0; i < numWorkers; i++) {
        datasetInitStartItem = startItem;
        if (i + 1 == numWorkers) {
            datasetInitItemCount = itemsPerThread + itemsRemainder;
        } else {
            datasetInitItemCount = itemsPerThread;
        }
        startItem += datasetInitItemCount;
        randomx_init_dataset(myDataset,myCache,datasetInitStartItem,datasetInitItemCount);
    }

    randomx_release_cache(myCache);
    myCache = NULL;

    struct param *parameters = (struct param *)malloc(sizeof(struct param));
    parameters->flags = flags_vm;
    parameters->cache = myCache;
    parameters->dataset = myDataset;
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
        frank_pthread_single_cpu_affinity_set(64-1-j, thread_id[j]); //绑核
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


// 指定线程绑核
int frank_pthread_single_cpu_affinity_set(int core_id, pthread_t tid)
{
    cpu_set_t mask;

    CPU_ZERO(&mask);
    CPU_SET(core_id, &mask);
    if (pthread_setaffinity_np(tid, sizeof(cpu_set_t), &mask) < 0)
    {
        fprintf(stderr, "set thread[%x] affinity failed\n", (unsigned int)tid);
        return 1;
    }

    return 0;
}