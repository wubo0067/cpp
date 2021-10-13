cmake_minimum_required (VERSION 3.18)
project (mypopen) 

set(CMAKE_C_COMPILER "/usr/bin/gcc")
set(CMAKE_CXX_COMPILER "/usr/bin/g++")

set(MYPOPEN_FILES
        main.c
        log.c
        log.h
        popen.c
        popen.h
        )

add_executable (mypopen ${MYPOPEN_FILES})