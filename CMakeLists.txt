cmake_minimum_required(VERSION 3.16)
project(rapid C)

set(CMAKE_C_STANDARD 11)

add_executable(rapid
        i3lock-fancy-rapid.c
        screenshot.h screenshot.c
        blur.c blur.h
        operations.h operations.c)

target_compile_options(rapid PRIVATE -fopenmp -Wall -Wextra)
target_link_libraries(rapid X11 -fopenmp png)
