project(hash-pool)
cmake_minimum_required(VERSION 3.10)

LINK_DIRECTORIES(~/sandbox/randomx-hash-tpool)

INCLUDE_DIRECTORIES(~/sandbox/randomx-hash-tpool)

add_executable(demo hash_pool.cpp)

target_link_libraries(demo librandomx.a)

set_property(TARGET demo PROPERTY CXX_STANDARD 11)