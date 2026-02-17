# Test executables and CTest registration.
# Include from root with option BUILD_TESTING (default ON).

add_executable(test_thread test_thread.c)
target_link_libraries(test_thread PRIVATE libgorak)
add_test(NAME test_thread COMMAND test_thread)

add_executable(test_fdset_input test_fdset_input.c)
target_link_libraries(test_fdset_input PRIVATE libgorak)
add_test(NAME test_fdset_input COMMAND test_fdset_input)

add_executable(test_queue_int test_queue_int.c)
target_link_libraries(test_queue_int PRIVATE libgorak)
add_test(NAME test_queue_int COMMAND test_queue_int)

add_executable(test_threadpool test_threadpool.c)
target_link_libraries(test_threadpool PRIVATE libgorak)
add_test(NAME test_threadpool COMMAND test_threadpool)
