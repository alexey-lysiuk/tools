cmake_minimum_required(VERSION 2.6)

project(gmkdump)

find_package(ZLIB)

if (NOT ZLIB_FOUND)
    set(ZLIB_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/zlib)
    add_subdirectory(${ZLIB_INCLUDE_DIR})
endif (NOT ZLIB_FOUND)

if (MSVC)
    add_definitions(-D_CRT_SECURE_NO_DEPRECATE)
    add_definitions(-D_CRT_NONSTDC_NO_DEPRECATE)
    add_definitions(-Dsnprintf=_snprintf)
endif (MSVC)

include_directories(${ZLIB_INCLUDE_DIR} decompiler)
file(GLOB SOURCE_FILES decompiler/*.cpp decompiler/*.hpp)

add_executable(gmkdump ${SOURCE_FILES})
target_link_libraries(gmkdump z)
