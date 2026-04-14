cmake_minimum_required(VERSION 3.10)
project(cmfnothingaudio)

add_library(cmfnothingaudio STATIC
    nothing.c
)
target_link_libraries(cmfnothingaudio PRIVATE pak)
