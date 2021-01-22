cmake_minimum_required(VERSION 3.17)
project(Block5 C)

set(CMAKE_C_STANDARD 99)

set(EXE_NAME ntpclient)

set(utils src/ntpcalculations.c src/ntplogic.c src/communication.c)

add_executable(${EXE_NAME} ntpclient.c ${utils})