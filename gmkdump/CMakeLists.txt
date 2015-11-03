cmake_minimum_required(VERSION 2.6)

project(gmkdump)

find_package(ZLIB)

if (NOT ZLIB_FOUND)
    set(ZLIB_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/zlib)
    add_subdirectory(${ZLIB_INCLUDE_DIR})
endif (NOT ZLIB_FOUND)

include_directories(${ZLIB_INCLUDE_DIR} decompiler)

add_executable(gmkdump
    decompiler/exe.cpp
    decompiler/gm80.cpp
    decompiler/gm81.cpp
    decompiler/gmk.cpp
    decompiler/main.cpp
    decompiler/obfuscation.cpp)

target_link_libraries(gmkdump zlib)
