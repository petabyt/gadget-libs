cmake_minimum_required(VERSION 3.10)
project(cmfnothingaudio)

if(MAKE_EXECUTABLE)
    add_executable(cmfnothingaudio nothing.c)
else()
    add_library(cmfnothingaudio SHARED nothing.c)
endif()
target_link_libraries(cmfnothingaudio PRIVATE pakrt)
