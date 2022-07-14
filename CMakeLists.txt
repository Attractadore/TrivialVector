cmake_minimum_required(VERSION 3.12)
project(TrivialVector LANGUAGES CXX)

add_library(TrivialVector INTERFACE include/Attractadore/TrivialVector.hpp)
target_include_directories(TrivialVector INTERFACE include)
target_compile_features(TrivialVector INTERFACE cxx_std_20)

add_library(Attractadore::TrivialVector ALIAS TrivialVector)

enable_testing()
add_subdirectory(test)
