cmake_minimum_required(VERSION 3.0)

project(bsputil LANGUAGES CXX)

include( CheckCXXSourceCompiles )

if (CMAKE_VERSION VERSION_LESS "3.1")
	set (CMAKE_CXX_FLAGS "-std=gnu++17 ${CMAKE_CXX_FLAGS}")
else ()
	set(CMAKE_CXX_STANDARD 17)
endif ()

CHECK_CXX_SOURCE_COMPILES("#include <ppl.h>
	int main() { concurrency::parallel_for(0, 1, 1, [](int) { } ); }"
	HAVE_PARALLEL_FOR)

if(NOT HAVE_PARALLEL_FOR)
	CHECK_CXX_SOURCE_COMPILES("#include <dispatch/dispatch.h>
		int main() { dispatch_apply(1, dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^(size_t) { }); }"
		HAVE_DISPATCH_APPLY)
endif()

if(HAVE_PARALLEL_FOR)
	add_definitions(-DHAVE_PARALLEL_FOR=1)
elseif(HAVE_DISPATCH_APPLY)
	add_definitions(-DHAVE_DISPATCH_APPLY=1)
else()
	option(NO_OPENMP "Disable usage of OpenMP" OFF)

	if(NOT NO_OPENMP)
		find_package(OpenMP)

		if(OPENMP_FOUND)
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
		endif()
	endif()
endif()

if(UNIX)
	add_definitions(-DLINUX)
endif()

add_definitions(-DERICWTOOLS_VERSION=v0.18.1-289-gaf7e330)

include_directories("${CMAKE_SOURCE_DIR}/src")

add_executable(${CMAKE_PROJECT_NAME}
	src/fmt/core.h
	src/fmt/format-inl.h
	src/fmt/format.cc
	src/fmt/format.h

	src/tbb/parallel_for.h

	src/common/bspfile.cc
	src/common/bspfile.hh
	src/common/bsputils.cc
	src/common/bsputils.hh
	src/common/cmdlib.cc
	src/common/cmdlib.hh
	src/common/entdata.cc
	src/common/entdata.h
	src/common/log.cc
	src/common/log.hh
	src/common/mathlib.cc
	src/common/mathlib.hh
	src/common/polylib.cc
	src/common/polylib.hh
	src/common/qvec.hh
	src/common/threads.cc
	src/common/threads.hh

	src/light/entities.hh
	src/light/imglib.hh
	src/light/light.hh
	src/light/litfile.hh
	src/light/ltface.hh
	src/light/settings.hh
	src/light/trace.hh

	src/bsputil.cc
	src/decompile.cpp
	src/decompile.h
)
