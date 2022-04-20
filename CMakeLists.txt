project(hash-pool)
cmake_minimum_required(VERSION 3.10)
add_subdirectory(lib/RandomX)

add_executable(demo hash_pool.c)
