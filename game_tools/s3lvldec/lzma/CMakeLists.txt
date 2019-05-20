cmake_minimum_required(VERSION 2.4)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_7ZIP_ST")

file(GLOB LZMA_FILES *.c *.h)

add_library( lzma ${LZMA_FILES} )
