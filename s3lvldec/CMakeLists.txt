cmake_minimum_required(VERSION 2.4)

project(s3lvldec)

if (MSVC)
    add_definitions(-D_CRT_SECURE_NO_DEPRECATE)
    add_definitions(-D_CRT_NONSTDC_NO_DEPRECATE)
endif (MSVC)

file(GLOB SOURCE_FILES *.cpp)

add_executable(s3lvldec ${SOURCE_FILES})
