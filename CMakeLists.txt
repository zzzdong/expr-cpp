cmake_minimum_required(VERSION 3.10.0)
project(expr VERSION 0.1.0 LANGUAGES C CXX)
find_package(GTest REQUIRED)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(include)
aux_source_directory(src SRC_LIST)

add_library(expr ${SRC_LIST})

include(CTest)
enable_testing()


add_executable(TestTokenizer tests/TestTokenizer.cpp)
target_link_libraries(TestTokenizer PRIVATE expr GTest::GTest GTest::Main)
add_test(TestTokenizer TestTokenizer)


add_executable(TestParser tests/TestParser.cpp)
target_link_libraries(TestParser PRIVATE expr GTest::GTest GTest::Main)
add_test(TestParser TestParser)

add_executable(TestEvaluator tests/TestEvaluator.cpp)
target_link_libraries(TestEvaluator PRIVATE expr GTest::GTest GTest::Main)
add_test(TestEvaluator TestEvaluator)
