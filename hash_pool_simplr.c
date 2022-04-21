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
            const unsigned char myKey[] = {255, 255,255, 254, 219, 155, 62, 29, 172, 210, 122, 149, 253, 169, 34, 24,
                                           33, 152, 221, 38, 200, 234, 74, 60, 118, 235, 15, 159, 33, 237, 210, 127};
            const unsigned char h0[] = {236,97,53,71,37,0,200,215,7,52,32,198,108,183,90,4,140,41,110,170,32,109,7,56,229,47,186,12,150,63,52,232};
            const unsigned char prevh[] = {61, 222, 227, 151, 197, 175, 127, 142, 18, 210, 148, 122, 239, 9, 40, 9, 78, 47, 1, 208, 199, 19, 214, 225, 211, 93, 196, 144, 253, 232, 176, 145, 62, 172, 183, 229, 89, 16, 42, 96, 247, 44, 228, 20, 71, 71, 31, 85};
            const unsigned char timestampBinary[] = {0,0,0,0,0,0,0,0,98,93,21,92};
            const unsigned char chunk[] = RANDOMX_HASH_TPOOL_CHUNK_AND_ENTROPY;
            const unsigned char entropy[] = RANDOMX_HASH_TPOOL_CHUNK_AND_ENTROPY;
            unsigned char myInput[] = {0};

            char hash[RANDOMX_HASH_SIZE];

            const int length_h0 = sizeof(h0)/sizeof(const unsigned char);
            const int length_prevh = sizeof(prevh)/sizeof(const unsigned char);
            const int length_time = sizeof(timestampBinary)/sizeof(const unsigned char);
            const int length_chunk = sizeof(chunk)/sizeof(const unsigned char);
            const int length_entropy = sizeof(entropy)/sizeof(const unsigned char);

            for (int i = 0; i < length_h0 + length_prevh + length_time + length_chunk + length_entropy ; i++)
            {
                if (i< length_h0)
                {
                myInput[i] = h0[i];
                }
                else if (i< length_h0 + length_prevh)
                {
                    myInput[i] = prevh[i - length_h0];
                }
                else if (i< length_h0 + length_prevh + length_time)
                {
                    myInput[i] = prevh[i - length_h0 - length_prevh];
                }
                else if (i< length_h0 + length_prevh + length_time + length_chunk)
                {
                    myInput[i] = prevh[i - length_h0 - length_time - length_chunk];
                }
                else if (i< length_h0 + length_prevh + length_time + length_chunk + length_entropy)
                {
                    myInput[i] = prevh[i - length_h0 - length_time - length_chunk - length_entropy];
                }

            }

            randomx_flags flags = randomx_get_flags();
            randomx_cache *myCache = randomx_alloc_cache(flags);
            randomx_init_cache(myCache, &myKey, sizeof myKey);
            randomx_vm *myMachine = randomx_create_vm(flags, myCache, NULL);

            randomx_calculate_hash(myMachine, &myInput, sizeof myInput, hash);

            randomx_destroy_vm(myMachine);
            randomx_release_cache(myCache);

            for (unsigned i = 0; i < RANDOMX_HASH_SIZE; ++i)
                printf("%02x\n", hash[i] & 0xff);

            printf("test done\n");

            return hash[1];


}