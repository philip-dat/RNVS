cmake_minimum_required(VERSION 3.17)
project(Block5 C)

set(CMAKE_C_STANDARD 99)

add_executable(Block5 ntpclient.c communication.c ntpcalculations.c communication.h ntpcalculations.h ntplogic.h ntplogic.c)