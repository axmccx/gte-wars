cmake_minimum_required(VERSION 3.25)

set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_LIST_DIR}/cmake/toolchain.cmake")

project(
    gte-wars
    LANGUAGES C CXX ASM
)

include(cmake/setup.cmake)
include(cmake/tools.cmake)

add_library(
    common OBJECT
    src/libc/clz.s
    src/libc/crt0.c
    src/libc/malloc.c
    src/libc/misc.c
    src/libc/setjmp.s
    src/libc/string.c
    src/libc/string.s
    src/ps1/cache.s
    src/vendor/printf.c
    src/controller.c
    src/ps1/gpu.c
    src/ps1/gte.c
    src/ps1/trig.c
    src/world.c
    src/model.c
    src/render.c
    src/rng.c
    src/font.c
)
target_compile_definitions(common PRIVATE PRINTF_DISABLE_SUPPORT_LONG_LONG)

target_include_directories(
    common PUBLIC
    src
    src/libc
)
link_libraries(common)

addPS1Executable(main src/main.c)
covertObj(
        objects/ship.obj
        objects/ship.dat
)
addBinaryFile(main playerShipObj "${PROJECT_BINARY_DIR}/objects/ship.dat")
covertObj(
        objects/bullet.obj
        objects/bullet.dat
)
addBinaryFile(main bulletObj "${PROJECT_BINARY_DIR}/objects/bullet.dat")
covertObj(
        objects/octahedron.obj
        objects/octahedron.dat
)
addBinaryFile(main octahedronObj "${PROJECT_BINARY_DIR}/objects/octahedron.dat")
convertImage(
        textures/font.png 4
        objects/fontTexture.dat
        objects/fontPalette.dat
)
addBinaryFile(main fontTexture "${PROJECT_BINARY_DIR}/objects/fontTexture.dat")
addBinaryFile(main fontPalette "${PROJECT_BINARY_DIR}/objects/fontPalette.dat")
