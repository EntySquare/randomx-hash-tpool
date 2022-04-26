//
// Created by luo2 on 2022/4/26.
//


#ifndef _CPU_H
#define _CPU_H


#ifdef __cplusplus
extern "C"
{
#endif

#define _GNU_SOURCE
#include <sched.h>
#include <pthread.h>

void print_cpu_affinity_id(int core_nums, pthread_t tid, cpu_set_t* cpu_set);
int frank_phtread_cpu_affinity_get(pthread_t tid, cpu_set_t* cpu_set);
int frank_pthread_single_cpu_affinity_set(int core_id, pthread_t tid);


#ifdef __cplusplus
}
#endif
#endif //_CPU_H