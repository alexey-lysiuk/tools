cmake_minimum_required(VERSION 2.6)

project(prettyzds)

find_package(ZLIB)

if (NOT ZLIB_FOUND)
    set(ZLIB_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/zlib)
    add_subdirectory(${ZLIB_INCLUDE_DIR})
endif (NOT ZLIB_FOUND)

if (MSVC)
    add_definitions(-D_CRT_SECURE_NO_DEPRECATE)
    add_definitions(-D_CRT_NONSTDC_NO_DEPRECATE)
endif (MSVC)

include_directories(${ZLIB_INCLUDE_DIR} minizip)

file(GLOB RAPIDJSON_FILES rapidjson/*.h)
file(GLOB MINIZIP_FILES minizip/*.c minizip/*.h)
set(SOURCE_FILES ${MINIZIP_FILES} ${RAPIDJSON_FILES} prettyzds.cpp)

add_executable(prettyzds ${SOURCE_FILES})
target_link_libraries(prettyzds z)
