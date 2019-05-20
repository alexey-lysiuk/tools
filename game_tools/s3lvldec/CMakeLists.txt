cmake_minimum_required(VERSION 2.4)

project(s3lvldec)

if (MSVC)
	add_definitions("-D_CRT_SECURE_NO_DEPRECATE -D_CRT_NONSTDC_NO_DEPRECATE")
elseif("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU" OR "${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wno-unused-variable")
endif (MSVC)

file(GLOB SOURCE_FILES *.cpp *.h)

add_subdirectory(lzma)

add_executable(s3lvldec ${SOURCE_FILES})
target_link_libraries(s3lvldec lzma)
