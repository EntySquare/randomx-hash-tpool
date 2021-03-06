//
// Created by luo2 on 2022/5/9.
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

#define CHUNK_ENTROPY_SIZE (256*1024)
#define THREADS_COUNT 3
#define LENGTH_PER_LIST 1000
#define LIST_NUM 1
#define numWorkers 191
int loop = 5;
int timing = 0;
long thread_ID = 0;

pthread_mutex_t main_lock ;
pthread_mutex_t ID_lock ;
pthread_mutex_t loop_lock[THREADS_COUNT] ;
pthread_mutex_t thread_lock[THREADS_COUNT] ;
// test lock
pthread_mutex_t cal_lock ;
pthread_cond_t cond_lock ;


static int validate_hash(
        unsigned char hash[RANDOMX_HASH_SIZE],
        unsigned char difficulty[RANDOMX_HASH_SIZE])
{
    return memcmp(hash, difficulty, RANDOMX_HASH_SIZE);
}


struct ids {
    int threads_id;
};

struct params {
    unsigned char* input;
    int inputSize;
    unsigned char* output;
    randomx_flags flags;
    randomx_cache *cache;
    randomx_dataset *dataset;
    unsigned char* diff;
} parameters[8];
//= malloc(sizeof(struct params));



//void hash_cal(randomx_vm *machine, const void *input, size_t inputSize, void *output)
void *hash_cal(void *paramsPtr)
{
    // binding the core
    cpu_set_t cpu_set;
    CPU_ZERO(&cpu_set);
    CPU_SET(((struct ids * )paramsPtr)->threads_id, &cpu_set);
    if (pthread_setaffinity_np(pthread_self(), sizeof(cpu_set),&cpu_set) < 0)
        perror("pthread_setaffinity_np");

    long tid = ((struct ids *) paramsPtr)->threads_id;
    printf("%ld Thread is created...\n", tid);
    pthread_mutex_lock(&thread_lock[tid]);
    pthread_mutex_lock(&ID_lock);
    thread_ID = tid;
    pthread_mutex_unlock(&main_lock);

    for(int lo = 0 ; lo < loop; lo++) {
        pthread_mutex_lock(&thread_lock[tid]);
        printf("%ld Thread starting task %d...\n", tid, lo+1);
        randomx_vm *myMachine = randomx_create_vm(((struct params *) parameters)->flags,
                                                  ((struct params *) parameters)->cache,
                                                  ((struct params *) parameters)->dataset);
        time_t start_total = time(NULL);
        time_t end_total;
        for (int k = 0; k < LIST_NUM; k++) {
            for (int m = 0; m < LENGTH_PER_LIST; m++) {
                // cal hash
                randomx_calculate_hash(myMachine, ((struct params *) parameters)->input,
                                       ((struct params *) parameters)->inputSize,
                                       ((struct params *) parameters)->output);
                // validate
                validate_hash(((struct params *) parameters)->output, ((struct params*)parameters)->diff);
            }
            if ((k + 1) == LIST_NUM ){
                unsigned char* hash = ((struct params*) parameters)->output;
                printf("\n");
                for (unsigned i = 0; i < RANDOMX_HASH_SIZE; ++i)
                { printf("%02x", hash[i] & 0xff); }
                printf("\n");
            }
        }
        printf("\n%ld Thread finish task %d ...\n", tid, lo+1);
        end_total = time(NULL);
        timing = timing + difftime(end_total, start_total);
        randomx_destroy_vm(myMachine);
        pthread_mutex_lock(&ID_lock);
        thread_ID = tid;
        pthread_mutex_unlock(&main_lock);
    }

    pthread_exit( (void*) paramsPtr);

}


void *cal_test(void *paramsPtr) {
    int cal_loop = 1;
    while(cal_loop <=3) {
        printf("\nwaiting...\n");
        pthread_mutex_lock(&cal_lock);
        pthread_cond_wait(&cond_lock, &cal_lock);
        printf("\ncal start...\n");
        double result = 0.0;
        for (int i = 0; i < 1000000; i++) {
            result = result + sin(i) * tan(i);
        }
        printf("\ncal done..., Result = %e\n", result);
        pthread_mutex_unlock(&cal_lock);
        cal_loop++;
    }
    pthread_exit((void*) paramsPtr);
}


// main
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
    unsigned char difficulty[] = {0,255,255,255,57,187,243,201,6,149,141,58,43,178,62,177,161,169,15,75,12,68,25,200,65,151,136,126,129,147,114,67};

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

    // read chunk
    FILE *chunk_file = NULL;
    unsigned char chunk_data[CHUNK_ENTROPY_SIZE];
    chunk_file = fopen( "/ardir/ar_chunk_storage1/10028580864000", "r+");
    int offset = 2097176000 - 1 ;
    fseek(chunk_file, 0, SEEK_END);  //locate at offset
    int nLen = ftell(chunk_file);   //get the whole length of the file
    printf("chunk file length is %d\n", nLen);
    fseek(chunk_file, offset+0, SEEK_SET);  //locate at offset
    int nRead = 10; //CHUNK_ENTROPY_SIZE ;
    fread(chunk_data, 1, nRead , chunk_file);
                for(int j=0; j<10; j++){
                    printf( "%d,", chunk_data[j]);}
                printf( "\n");
    fclose(chunk_file);

    // string test
    char input[4][20]={"/ardir/chunk1", "/ardir/chunk2", "/ardir/chunk3","/ardir/chunk4" };
    char FilePath[2][20];
    strcpy(FilePath[1], input[3]);
    printf("%s\n", FilePath[1]);

    // binary test
    char data[4][4]={{1,2,3}, {4,5,6}, {0,11,13,6}, {9,7,6,0}};
    char size[4]={3, 4, 5, 6};
    char fetch_data[2][4];
    long fetch_size[4];
    memcpy(fetch_data[1], data[3], 4);
    memcpy(&fetch_size[3], &size[3], 1);
    for(int i =0 ; i<3; i++) printf("%d,", fetch_data[1][i]);
    printf("\n");
    printf("size is %ld \n", fetch_size[1]);

    //end intervalstart test
    long byte = 200447320910;
    FILE *chunk_storage_file = NULL;
    chunk_storage_file = fopen("/ardir/enty_ar_chunk_storage.dat", "r+");
    printf("\nenty_ar_chunk_storage is open\n");
    fseek(chunk_storage_file, 0, SEEK_END);
    long nLen1 = ftell(chunk_storage_file);
    for(long i = 0 ; i < nLen1 ; i++) {
        unsigned char EndIntervalStart[2];
        fseek(chunk_storage_file, i, SEEK_SET);  // read from position
        fread(EndIntervalStart, 1, 1, chunk_storage_file);
        //if (EndIntervalStart[0] >= byte && EndIntervalStart[1] <= byte){
            printf("IntervalStart is %ld\n", EndIntervalStart[1]);
        //break;}
    }
    printf("intervalstart is found\n");

    // condition lock
    pthread_mutex_init(&cal_lock, NULL);
    pthread_cond_init(&cond_lock, NULL);
    pthread_t cal_id;
    pthread_create(&cal_id, NULL, cal_test, NULL);
    int loop = 1;
    while(loop <= 3){
    pthread_cond_signal(&cond_lock);
    loop ++;
    }

    // init dataset
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
    parameters->flags = flags_vm;
    parameters->cache = myCache;
    parameters->dataset = myDataset;
    parameters->diff = difficulty;

    pthread_t thread_id[THREADS_COUNT], fetch_thread;
    for (long j = 0; j < THREADS_COUNT; j++) {
        pthread_mutex_init(&thread_lock[j], NULL);}
    pthread_mutex_init(&main_lock, NULL);
    pthread_mutex_lock(&main_lock);
    pthread_mutex_init(&ID_lock, NULL);
    printf("mutex lock is initiated\n");

    for (long j = 0; j < THREADS_COUNT; j++) {
        struct ids *init = (struct ids *) malloc(sizeof(struct ids));
        init->threads_id = j;
        pthread_create(&thread_id[j], NULL, hash_cal, (void *) init);
        //printf("threads %ld is created\n", j+1);
    }
    sleep(1);

    int l = 0 ;
    int loop_test = (loop + 1) * THREADS_COUNT;
    while ( l < loop_test ){
        printf("main thread waiting to be unlocked\n");
        pthread_mutex_lock(&main_lock);
        parameters->input = myInput;
        parameters->inputSize = sizeof myInput;
        parameters->output = hash;
        pthread_mutex_unlock(&thread_lock[thread_ID]);
        pthread_mutex_unlock(&ID_lock);
        l++;
    }

    for (long k = 0; k<THREADS_COUNT ; k++){
        pthread_join(thread_id[k], NULL);
    }

    printf("\nthe parallel calc rate is %d h/s\n", (loop * LENGTH_PER_LIST * LIST_NUM * THREADS_COUNT / timing));

//    randomx_calculate_hash(myMachine, &myInput, sizeof myInput, hash);
//    for (unsigned i = 0; i < RANDOMX_HASH_SIZE; ++i)
//        printf("%02x", hash[i] & 0xff);

    printf("\ntest done\n\n");

}
