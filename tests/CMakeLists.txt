set(LGK_TESTS thread fdset_input queue_int threadpool timespec_ms bitmap fsm)

foreach(name IN LISTS LGK_TESTS)
    add_executable(test_${name} test_${name}.c)
    target_link_libraries(test_${name} PRIVATE libgorak)
    add_test(NAME test_${name} COMMAND test_${name})
endforeach()
