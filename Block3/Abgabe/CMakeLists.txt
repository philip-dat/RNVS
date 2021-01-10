cmake_minimum_required(VERSION 3.17)
project(Block3 C)

set(CMAKE_C_STANDARD 99)

add_executable(client client.c communication.h uthash.h)
add_executable(server server.c communication.h uthash.h)

