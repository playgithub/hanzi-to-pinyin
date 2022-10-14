add_executable(test_case_1 test/test_case_1.cpp)

target_link_libraries(test_case_1 ${PROJECT_NAME})

add_test(NAME test_case_1
         COMMAND test_case_1
         WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
