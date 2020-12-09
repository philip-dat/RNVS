cmake_minimum_required(VERSION 3.5.1)
project(Block2 C)

set(CMAKE_C_STANDARD 99)

add_executable(Block2-client client.c)
add_executable(Block2-server server.c)

set_target_properties(Block2-client PROPERTIES OUTPUT_NAME "client")
set_target_properties(Block2-server PROPERTIES OUTPUT_NAME "server")
