//
// Created by terrill on 2022/4/21.
//

#include "randomx.h"
#include "chunk_and_entropy.h"
#include <stdio.h>

//int main() {
////    const char myKey[] = "RandomX example key";
////    const char myInput[] = "RandomX example input";
////    char hash[RANDOMX_HASH_SIZE];
////
////    randomx_flags flags = randomx_get_flags();
////    randomx_cache *myCache = randomx_alloc_cache(flags);
////    randomx_init_cache(myCache, &myKey, sizeof myKey);
////    randomx_vm *myMachine = randomx_create_vm(flags, myCache, NULL);
////
////    randomx_calculate_hash(myMachine, &myInput, sizeof myInput, hash);
////
////    randomx_destroy_vm(myMachine);
////    randomx_release_cache(myCache);
////
////    for (unsigned i = 0; i < RANDOMX_HASH_SIZE; ++i)
////        printf("%02x\n", hash[i] & 0xff);
////
////    printf("test done\n");
////
////    return 0;
//}

int main()
{
            const int myKey[] = {255, 255,255, 254, 219, 155, 62, 29, 172, 210, 122, 149, 253, 169, 34, 24,
                                           33, 152, 221, 38, 200, 234, 74, 60, 118, 235, 15, 159, 33, 237, 210, 127};
            const int h0[] = {236,97,53,71,37,0,200,215,7,52,32,198,108,183,90,4,140,41,110,170,32,109,7,56,229,47,186,12,150,63,52,232};
            const int prevh[] = {61, 222, 227, 151, 197, 175, 127, 142, 18, 210, 148, 122, 239, 9, 40, 9, 78, 47, 1, 208, 199, 19, 214, 225, 211, 93, 196, 144, 253, 232, 176, 145, 62, 172, 183, 229, 89, 16, 42, 96, 247, 44, 228, 20, 71, 71, 31, 85};
            const int timestampBinary[] = {0,0,0,0,0,0,0,0,98,93,21,92};
            const int chunk[] = RANDOMX_HASH_TPOOL_CHUNK_AND_ENTROPY;
            const int entropy[] = RANDOMX_HASH_TPOOL_CHUNK_AND_ENTROPY;
             int myInput[] = {0};
            char hash[RANDOMX_HASH_SIZE];

            int len_h0 = sizeof(h0)/sizeof(int);
            int len_prevh = sizeof(prevh)/sizeof(int);
            int len_time = sizeof(timestampBinary)/sizeof(int);
            int len_chunk = sizeof(chunk)/sizeof(int);
            int len_entropy = sizeof(entropy)/sizeof(int);

    printf("input ready ==, length is %d\n", len_chunk);

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


            randomx_flags flags = randomx_get_flags();
//            randomx_flags flags= RANDOMX_FLAG_FULL_MEM;
//            flags |= RANDOMX_FLAG_HARD_AES;
//            flags |= RANDOMX_FLAG_JIT;
//            flags |= RANDOMX_FLAG_LARGE_PAGES;

            randomx_cache *myCache = randomx_alloc_cache(flags);
            randomx_init_cache(myCache, &myKey, sizeof myKey);
            randomx_vm *myMachine = randomx_create_vm(flags, myCache, NULL);

            randomx_calculate_hash(myMachine, &myInput, sizeof myInput, hash);

            randomx_destroy_vm(myMachine);
            randomx_release_cache(myCache);

            for (unsigned i = 0; i < RANDOMX_HASH_SIZE; ++i)
                printf("%02x", hash[i] & 0xff);

            printf("\ntest done\n");

            return hash[1];


}