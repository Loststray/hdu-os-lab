cmake_minimum_required(VERSION 3.15)
project("HDU OS LABS")

set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Werror")

add_subdirectory("lab3")
add_subdirectory("lab5")

add_custom_target(
    format
    COMMAND ${CMAKE_COMMAND} -E echo "formatting\n"
    find ../ -path ../build -prune  -o -type f -name "*.c" -exec clang-format -i {} "\;"
)
add_custom_target(
    run_shell
    ./lab3/myshell
)