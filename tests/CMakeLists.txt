# Test executables and CTest registration.
# Include from root with option BUILD_TESTING (default ON).

add_executable(test_thread test_thread.c)
target_link_libraries(test_thread PRIVATE libgorak)
add_test(NAME test_thread COMMAND test_thread)

add_executable(test_fdset_input test_fdset_input.c)
target_link_libraries(test_fdset_input PRIVATE libgorak)
add_test(NAME test_fdset_input COMMAND test_fdset_input)
