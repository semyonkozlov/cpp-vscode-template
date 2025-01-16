include(FetchContent)

FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
FetchContent_MakeAvailable(googletest)

# some function to easily add tests
function(addTest TEST_NAME TEST_SOURCE)
  add_executable(${TEST_NAME} ${TEST_SOURCE})
  target_link_libraries(${TEST_NAME} gtest_main)
  add_test(${TEST_NAME} ${TEST_NAME}) # simply add args if you need some
endfunction()

# the tests
addTest(SomeTest SomeTest.cpp)