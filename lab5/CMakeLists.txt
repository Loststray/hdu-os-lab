cmake_minimum_required(VERSION 3.15)
project("HDU OS LAB5" CXX)

set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -Wshadow -Wconversion")

add_executable(FAT ./FAT.cpp)
