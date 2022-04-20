project(hash-pool)
cmake_minimum_required(VERSION 3.10)

LINK_DIRECTORIES(~/sandbox/randomx-hash-tpool)

add_executable(demo hash_pool.c)

target_link_libraries(demo librandomx.a)