cmake_minimum_required(VERSION 3.15)
project("HDU OS LAB5" C)

set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wshadow -Wconversion")

include_directories("./include")
add_executable(FAT ./FAT.c)
