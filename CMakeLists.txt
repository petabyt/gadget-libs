cmake_minimum_required(VERSION 3.10)
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
project(pak-modules)

option(MAKE_EXECUTABLE "" ON)
set(LIBPAK_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/../libpak/include)

add_library(pak INTERFACE)
target_include_directories(pak INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/../libpak/include)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/goveelife ${CMAKE_CURRENT_BINARY_DIR}/goveelife)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/dummy ${CMAKE_CURRENT_BINARY_DIR}/dummy)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/nothing-buds ${CMAKE_CURRENT_BINARY_DIR}/nothing-buds)
