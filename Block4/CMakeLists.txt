cmake_minimum_required(VERSION 3.14)
project(Block_4 C)

set(CMAKE_C_STANDARD 99)

add_executable(peer peer.c uthash.h lookup.h peer.h client_request.h communication.h)
add_executable(client client.c uthash.h communication.h)
add_compile_options(-g)