project(hash-pool)
cmake_minimum_required(VERSION 3.10)

LINK_DIRECTORIES(~/sandbox/randomx-hash-tpool)
LINK_DIRECTORIES(~/sandbox/randomx-hash-tpool)



INCLUDE_DIRECTORIES(/usr/lib/x86_64-linux-gnu)

add_executable(demo hash_pool.cpp hash_pool_pthread_test.c)
TARGET_LINK_LIBRARIES(demo pthread)
target_link_libraries(demo librandomx.a)

set_property(TARGET demo PROPERTY CXX_STANDARD 11)

if(NOT Threads_FOUND AND UNIX AND NOT APPLE)
    set(THREADS_PREFER_PTHREAD_FLAG ON)
    find_package(Threads)
endif()