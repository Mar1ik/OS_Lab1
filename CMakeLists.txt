cmake_minimum_required(VERSION 3.27)
project(untitled10 C)

set(CMAKE_C_STANDARD 11)

add_executable(untitled10 main.c
        shell.c
        ema-replace-int_ema-replace-int_Version1.c
        ema-replace-int_ema-gen-data_Version1.c
        cpu-calc-md5_cpu-calc-md5_Version1.c
        cpu-calc-md5_cpu-calc-md5-mt_Version1.c)
