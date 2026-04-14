cmake_minimum_required(VERSION 3.10)
project(dummy)

add_library(dummy STATIC
    main.c
)
target_link_libraries(dummy PRIVATE pak)
