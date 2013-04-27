enable_testing()

add_executable(test_OCBase test_OCBase.c)
find_package(Check REQUIRED)
if(CHECK_FOUND)
  include_directories(${CHECK_INCLUDE_DIR})
  target_link_libraries(test_OCBase ${CHECK_LIBRARY})
endif(CHECK_FOUND)
target_link_libraries(test_OCBase OCBase)

add_test(test_OCBase ${CMAKE_CURRENT_BINARY_DIR}/test_OCBase)
