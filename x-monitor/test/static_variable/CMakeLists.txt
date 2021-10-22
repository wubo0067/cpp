AUX_SOURCE_DIRECTORY(. test_static_variable)
add_executable(test_static_variable ${test_static_variable})
target_link_libraries(test_static_variable xmonitor-utils pthread z m)