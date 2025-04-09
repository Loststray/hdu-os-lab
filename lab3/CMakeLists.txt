cmake_minimum_required(VERSION 3.15)
project("HDU OS LAB3")

set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Werror")


add_executable(myshell ${CMAKE_SOURCE_DIR}/lab3/myshell.c)
add_executable(pipe ${CMAKE_SOURCE_DIR}/lab3/pipe/pipe.c)
add_executable(shared_memory ${CMAKE_SOURCE_DIR}/lab3/shared_memory/shared_memory.c)
add_executable(info_queue ${CMAKE_SOURCE_DIR}/lab3/info_queue/info_queue.c)
