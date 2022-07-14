if (BUILD_TESTING)
find_package(GTest REQUIRED)
include(GoogleTest)

add_executable(TestTrivialVector TestTrivialVector.cpp)
target_link_libraries(TestTrivialVector GTest::gtest_main Attractadore::TrivialVector)

gtest_discover_tests(TestTrivialVector)
endif()
