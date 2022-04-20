//
// Created by terrill on 2022/4/20.
//

#include "lib/RandomX/src/randomx.h"
#include <stdio.h>
#include <list>

static ERL_NIF_TERM randomx_hash_thread_nif( const void key, const void hashData, const void num)

{
    int main() {
        const char myKey[] = key; //"RandomX example key";
        const char myInput[] = hashData; //"RandomX example input";
        const char RANDOMX_HASH_SIZE = 10;
        char hash[RANDOMX_HASH_SIZE];
        list<int> l1;

        randomx_cache *myCache = randomx_alloc_cache(RANDOMX_FLAG_DEFAULT);
        randomx_init_cache(myCache, &myKey, sizeof myKey);
        randomx_vm *myMachine = randomx_create_vm(RANDOMX_FLAG_DEFAULT, myCache, NULL);

        randomx_calculate_hash(myMachine, &myInput, sizeof myInput, hash);

        randomx_destroy_vm(myMachine);
        randomx_release_cache(myCache);

        for (unsigned i = 0; i < RANDOMX_HASH_SIZE; ++i){
            printf("%02x", hash[i] & 0xff);
        printf("\n");
        l1.insert(i, hash[i]);
        }
        return l1;
        }
}