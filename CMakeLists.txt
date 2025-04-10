cmake_minimum_required(VERSION 3.15)
project("HDU OS LABS")

set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Werror")

add_subdirectory("lab3")
add_subdirectory("lab5")

add_custom_target(
    format
    # find ../ -path ../build -prune -o -type f -name "*.c" -o -name "*.cpp" -o -name "*.hpp" -exec clang-format -i {} "\;"
    clang-format -i ../lab3/*.c &&
    clang-format -i ../lab3/*/*.c &&
    clang-format -i ../lab5/*.cpp &&
    clang-format -i ../lab5/*.hpp
)
add_custom_target(
    run_shell
    ./lab3/myshell
)