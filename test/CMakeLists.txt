if (BUILD_TESTING)
find_package(GTest REQUIRED)
include(GoogleTest)

add_executable(TestTrivialVector TestTrivialVector.cpp)
target_link_libraries(TestTrivialVector GTest::gtest_main Attractadore::TrivialVector)

gtest_discover_tests(TestTrivialVector)

find_package(benchmark)
if (TARGET benchmark::benchmark)
    add_executable(BenchPushBack BenchPushBack.cpp)
    target_link_libraries(BenchPushBack benchmark::benchmark Attractadore::TrivialVector)

    add_executable(BenchAssign BenchAssign.cpp)
    target_link_libraries(BenchAssign benchmark::benchmark Attractadore::TrivialVector)
endif()
endif()
