add_executable(parsing_test parsing_test.c)
target_link_libraries(parsing_test parsing ADT)

add_executable(client_server_test client_server_test.c)
target_link_libraries(client_server_test server parsing ADT dl)

add_executable(adt_list_test adt_list_test.c)
target_link_libraries(adt_list_test parsing server ADT dl)

install(PROGRAMS ${CMAKE_CURRENT_SOURCE_DIR}/parsing_test_cmake.sh DESTINATION ${TESTS_PATH})
install(PROGRAMS ${EXECUTABLE_OUTPUT_PATH}/parsing_test DESTINATION ${TESTS_PATH})
install(PROGRAMS ${EXECUTABLE_OUTPUT_PATH}/adt_list_test DESTINATION ${TESTS_PATH})

set(SAPO_PATH ${CMAKE_CURRENT_SOURCE_DIRC}/parser_examples})

add_test(NAME test_sapo1_ok COMMAND ${TESTS_PATH}/parsing_test_cmake.sh 1 ok ${SAPO_PATH})
add_test(NAME test_sapo2_ok COMMAND ${TESTS_PATH}/parsing_test_cmake.sh 2 ok ${SAPO_PATH})
add_test(NAME test_sapo3_ok COMMAND ${TESTS_PATH}/parsing_test_cmake.sh 3 ok ${SAPO_PATH})
add_test(NAME test_sapo1_err COMMAND ${TESTS_PATH}/parsing_test_cmake.sh 1 err ${SAPO_PATH})
add_test(NAME test_sapo2_err COMMAND ${TESTS_PATH}/parsing_test_cmake.sh 2 err ${SAPO_PATH})
add_test(NAME test_sapo3_err COMMAND ${TESTS_PATH}/parsing_test_cmake.sh 3 err ${SAPO_PATH})
add_test(NAME test_sapo4_err COMMAND ${TESTS_PATH}/parsing_test_cmake.sh 4 err ${SAPO_PATH})
add_test(NAME test_sapo5_err COMMAND ${TESTS_PATH}/parsing_test_cmake.sh 5 err ${SAPO_PATH})
add_test(NAME test_sapo6_err COMMAND ${TESTS_PATH}/parsing_test_cmake.sh 6 err ${SAPO_PATH})
add_test(NAME test_sapo7_err COMMAND ${TESTS_PATH}/parsing_test_cmake.sh 7 err ${SAPO_PATH})
