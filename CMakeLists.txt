cmake_minimum_required(VERSION 3.25)

set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_LIST_DIR}/cmake/toolchain.cmake")

project(
    ps1-poc
    LANGUAGES C CXX ASM
)

include(cmake/setup.cmake)
include(cmake/tools.cmake)

add_library(
    common OBJECT
    src/libc/clz.s
    src/libc/crt0.c
    src/libc/cxxsupport.cpp
    src/libc/malloc.c
    src/libc/misc.c
    src/libc/setjmp.s
    src/libc/string.c
    src/libc/string.s
    src/ps1/cache.s
    src/vendor/printf.c
)
target_compile_definitions(common PRIVATE PRINTF_DISABLE_SUPPORT_LONG_LONG)

target_include_directories(
    common PUBLIC
    src
    src/libc
)
link_libraries(common)

addPS1Executable(main src/main.c)