cmake_minimum_required(VERSION 3.10)
project(goveelife)

if(MAKE_EXECUTABLE)
    add_executable(goveelife main.c)
else()
    add_library(goveelife STATIC
        main.c
    )
endif()
target_link_libraries(goveelife PRIVATE pak)
