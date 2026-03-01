cmake_minimum_required(VERSION 3.18)

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

set(LIBGORAK_SOURCES ${LIBGORAK_SOURCES} "lgk_util.h")
set(LIBGORAK_SOURCES ${LIBGORAK_SOURCES} "lgk_tnt.h" "tnt.c" "trace_print_default.c" "trace_print_default_nolevel.c")
set(LIBGORAK_SOURCES ${LIBGORAK_SOURCES} "lgk_char.h" "char.c")
set(LIBGORAK_SOURCES ${LIBGORAK_SOURCES} "lgk_array.h" "array_find_int.c" "array_fill_int.c")
set(LIBGORAK_SOURCES ${LIBGORAK_SOURCES} "lgk_date.h" "date_misc.c")
set(LIBGORAK_SOURCES ${LIBGORAK_SOURCES} "lgk_timespec.h" "timespec_ms.c")
set(LIBGORAK_SOURCES ${LIBGORAK_SOURCES} "lgk_fd.h" "fd_read_timed.c" "fd_write_timed.c" "fd_read_loop.c" "fd_write_loop.c")
set(LIBGORAK_SOURCES ${LIBGORAK_SOURCES} "lgk_queue.h")
set(LIBGORAK_SOURCES ${LIBGORAK_SOURCES} "lgk_queue_ptr.h" "queue_ptr.c")
set(LIBGORAK_SOURCES ${LIBGORAK_SOURCES} "lgk_queue_int.h" "queue_int.c")
set(LIBGORAK_SOURCES ${LIBGORAK_SOURCES} "lgk_queue_uint.h" "queue_uint.c")
set(LIBGORAK_SOURCES ${LIBGORAK_SOURCES} "lgk_threadpool.h" "threadpool.c")
set(LIBGORAK_SOURCES ${LIBGORAK_SOURCES} "lgk_threads.h" "mtx_timedlock_ms.c" "mtx_timedlock_ts.c" "cnd_timedwait_ms.c" "cnd_timedwait_ts.c" "monitor.c" "thread.c" "thrdstrerror.c")
set(LIBGORAK_SOURCES ${LIBGORAK_SOURCES} "lgk_fdset_input.h" "fdset_input.c")

add_library(libgorak ${LIBGORAK_SOURCES})
target_include_directories(libgorak PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# remove "lib" prefix since it's already part of the project name
set_target_properties(libgorak PROPERTIES PREFIX "")

# Tests (optional, enabled by default); use ctest to run
option(BUILD_TESTING "Build and enable tests (ctest)" ON)
if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()
