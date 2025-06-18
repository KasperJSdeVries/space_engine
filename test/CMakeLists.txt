find_package(CMocka REQUIRED)

option(DO_MEMCHECK "Use valgrind to check tests for memory leaks" OFF)
if(DO_MEMCHECK)
  find_program(VALGRIND valgrind)
endif()

file(GLOB TEST_SOURCES CONFIGURE_DEPENDS "*.c")
foreach(test_src ${TEST_SOURCES})
  get_filename_component(test_name ${test_src} NAME_WE)
  add_executable(${test_name} ${test_src})
  target_link_libraries(${test_name} PRIVATE engine cmocka::cmocka)

  if(DO_MEMCHECK)
    add_test(NAME ${test_name}
             COMMAND ${VALGRIND} --tool=memcheck --leak-check=full
                     $<TARGET_FILE:${test_name}>)
    set_tests_properties(${test_name} PROPERTIES FAIL_REGULAR_EXPRESSION
                                                 "ERROR SUMMARY: [^0]")
  else()
    add_test(NAME ${test_name} COMMAND ${test_name})
  endif()
endforeach(test_src)
