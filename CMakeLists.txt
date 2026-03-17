cmake_minimum_required(VERSION 3.23)

include(CMakeDependentOption)

set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)
set(CMAKE_C_STANDARD 23)
set(CMAKE_C_EXTENSIONS FALSE)
set(CMAKE_C_STANDARD_REQUIRED TRUE)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)

project(libgorak LANGUAGES C)

if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
    add_compile_options(-Wall -Wextra -pedantic)
    add_compile_options(-O3 -ffast-math)
    add_compile_options(-march=native)
else()
    message(WARNING "please add optimization flags for your compiler (\"${CMAKE_C_COMPILER_ID}\")")
endif()

add_library(libgorak)

target_sources(libgorak PUBLIC
  FILE_SET headers TYPE HEADERS
  BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
  FILES
    include/lgk/array.h
    include/lgk/bitmap.h
    include/lgk/char.h
    include/lgk/date.h
    include/lgk/fd.h
    include/lgk/fdset_input.h
    include/lgk/fsm.h
    include/lgk/queue.h
    include/lgk/queue_int.h
    include/lgk/queue_ptr.h
    include/lgk/queue_uint.h
    include/lgk/threadpool.h
    include/lgk/threads.h
    include/lgk/timespec.h
    include/lgk/tnt.h
    include/lgk/util.h
)

target_sources(libgorak PRIVATE
  src/array_fill_int.c
  src/array_find_int.c
  src/bitmap.c
  src/char.c
  src/cnd_timedwait_ms.c
  src/cnd_timedwait_ts.c
  src/date_misc.c
  src/fd_read_loop.c
  src/fd_read_timed.c
  src/fd_write_loop.c
  src/fd_write_timed.c
  src/fdset_input.c
  src/monitor.c
  src/mtx_timedlock_ms.c
  src/mtx_timedlock_ts.c
  src/queue_int.c
  src/queue_ptr.c
  src/queue_uint.c
  src/thrdstrerror.c
  src/thread.c
  src/threadpool.c
  src/timespec_ms.c
  src/tnt.c
  src/trace_print_default.c
  src/trace_print_default_nolevel.c
)

# remove "lib" prefix since it's already part of the project name
set_target_properties(libgorak PROPERTIES PREFIX "")

# Tests (optional, enabled by default); use ctest to run
option(BUILD_TESTING "Build and enable tests (ctest)" ON)
if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()
