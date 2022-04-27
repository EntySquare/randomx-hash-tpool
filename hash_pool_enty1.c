//
// Created by luo2 on 2022/4/27.
//

#define _GNU_SOURCE
#include <sched.h>
#include "randomx.h"
#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>


#define THREADS_COUNT 5
#define LENGTH_PER_LIST 2000
#define LIST_NUM 3
#define numWorkers 191

int timing=0;
pthread_mutex_t loop_lock[THREADS_COUNT] ;
pthread_mutex_t mutex[THREADS_COUNT] ;

static int validate_hash(
        unsigned char hash[RANDOMX_HASH_SIZE],
        unsigned char difficulty[RANDOMX_HASH_SIZE])
{
    return memcmp(hash, difficulty, RANDOMX_HASH_SIZE);
}

struct param {
    int threads_id;
};

struct param1 {
    int tasks_id;
    randomx_flags flags;
    randomx_cache *cache;
    randomx_dataset *dataset;
    unsigned char* input;
    int inputSize;
    unsigned char* output;
} parameters[7];
//= malloc(sizeof(struct param1));

//void hash_cal(randomx_vm *machine, const void *input, size_t inputSize, void *output)
void *hash_cal(void *paramsPtr)
{
    // binding the core
    cpu_set_t cpu_set;
    CPU_ZERO(&cpu_set);
    CPU_SET(((struct param*)paramsPtr)->threads_id, &cpu_set);
    if (pthread_setaffinity_np(pthread_self(), sizeof(cpu_set),&cpu_set) < 0)
        perror("pthread_setaffinity_np");

    long tid = ((struct param *) paramsPtr)->threads_id;

    for(int lo = 0 ; lo < 3; lo++) {

        pthread_mutex_lock(&mutex[tid]);
        if (lo ==0) {printf("%ld Thread is created...\n", tid);}
        else {
            long task = ((struct param1 *) parameters)->tasks_id;
            printf("%ld Thread starting task %ld...\n", tid, task);
            randomx_vm *myMachine = randomx_create_vm(((struct param1 *) parameters)->flags,
                                                      ((struct param1 *) parameters)->cache,
                                                      ((struct param1 *) parameters)->dataset);

            time_t start = time(NULL);
            time_t start_total = time(NULL);
            time_t end, end_total;

            for (int k = 0; k < LIST_NUM; k++) {
                for (int m = 0; m < LENGTH_PER_LIST; m++) {
                    randomx_calculate_hash(myMachine, ((struct param1 *) parameters)->input,
                                           ((struct param1 *) parameters)->inputSize,
                                           ((struct param1 *) parameters)->output);

                }
        if ((k + 1) == LIST_NUM * LENGTH_PER_LIST ){
            unsigned char* hash = ((struct param1*) parameters)->output;
            for (unsigned i = 0; i < RANDOMX_HASH_SIZE; ++i)
            { printf("%02x", hash[i] & 0xff); }
            printf("\n");
        }

            }
            printf("%ld Thread finish task %ld ...\n", tid, task);
            end_total = time(NULL);
            timing = timing + difftime(end_total, start_total);
            randomx_destroy_vm(myMachine);

            pthread_mutex_lock(&loop_lock[tid]);
        }
    }

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
    //unsigned char preimage[LIST_NUM * LENGTH_PER_LIST][] =
    int lem = sizeof(myInput);
    printf("myinput data size is %d\n", lem);

    randomx_flags flags_vm = RANDOMX_FLAG_FULL_MEM;
    flags_vm |= RANDOMX_FLAG_HARD_AES;
    flags_vm |= RANDOMX_FLAG_JIT;
    flags_vm |= RANDOMX_FLAG_LARGE_PAGES;
    flags_vm |= RANDOMX_FLAG_ARGON2_SSSE3;
    randomx_flags flags_fast = RANDOMX_FLAG_DEFAULT;
    flags_fast |= RANDOMX_FLAG_JIT;
    printf("flags is %d\n", flags_vm);
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

    pthread_t thread_id[THREADS_COUNT];
    for (long j = 0; j < THREADS_COUNT; j++) {
        pthread_mutex_init(&mutex[j], NULL);
        pthread_mutex_init(&loop_lock[j], NULL);}
    printf("mutex lock is initiated\n");

    for (long j = 0; j < THREADS_COUNT; j++) {
        struct param *init = (struct param *) malloc(sizeof(struct param));
        init->threads_id = j;
        pthread_create(&thread_id[j], NULL, hash_cal, (void *) init);
        //printf("threads %ld is created\n", j+1);
    }

    int loop = 2;
    for (long l = 0; l<loop ; l++) {
        for (long j = 0; j < THREADS_COUNT; j++) {
            if (l>0) {printf("waiting to be unlocked\n");}
            pthread_mutex_lock(&loop_lock[j]);
            parameters->flags = flags_vm;
            parameters->cache = myCache;
            parameters->dataset = myDataset;
            parameters->input = myInput;
            parameters->inputSize = sizeof myInput;
            parameters->output = hash;
            parameters->tasks_id = l + 1;
            pthread_mutex_unlock(&mutex[j]);
        }
    }

    for (long k = 0; k<THREADS_COUNT ; k++){
        pthread_join(thread_id[k], NULL);
    }

    printf("the parallel calc rate is %d h/s\n", (loop*LENGTH_PER_LIST * LIST_NUM * THREADS_COUNT / timing));

//    randomx_calculate_hash(myMachine, &myInput, sizeof myInput, hash);
//    for (unsigned i = 0; i < RANDOMX_HASH_SIZE; ++i)
//        printf("%02x", hash[i] & 0xff);

    if(validate_hash(hash, difficulty)>0)
    { printf("\nsolution found\n");}
    else
    { printf("\nsolution unfound\n");}

    printf("\ntest done\n");

}
