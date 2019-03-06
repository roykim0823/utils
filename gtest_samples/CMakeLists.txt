cmake_minimum_required(VERSION 3.2)
project(gtest_sample)

set(CMAKE_CXX_FLAGS "-std=c++11 -g -Wall -Wextra -pthread")
cmake_policy(SET CMP0048 NEW)

find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})
#find_package(GTest_main REQUIRED)
#pkg_search_module(GTEST REQUIRED gtest_main)
set(GTEST_MAIN "/usr/lib/libgtest_main.a")

include_directories(include)

add_executable(sample1_unittest unittest/sample1_unittest.cc src/sample1.cc)
target_link_libraries(sample1_unittest ${GTEST_LIBRARIES} ${GTEST_MAIN})

add_executable(sample2_unittest unittest/sample2_unittest.cc src/sample2.cc)
target_link_libraries(sample2_unittest ${GTEST_LIBRARIES} ${GTEST_MAIN})

add_executable(sample3_unittest unittest/sample3_unittest.cc)
target_link_libraries(sample3_unittest ${GTEST_LIBRARIES} ${GTEST_MAIN})

add_executable(sample4_unittest unittest/sample4_unittest.cc src/sample4.cc)
target_link_libraries(sample4_unittest ${GTEST_LIBRARIES} ${GTEST_MAIN})

add_executable(sample5_unittest unittest/sample5_unittest.cc src/sample1.cc)
target_link_libraries(sample5_unittest ${GTEST_LIBRARIES} ${GTEST_MAIN})

add_executable(sample6_unittest unittest/sample6_unittest.cc)
target_link_libraries(sample6_unittest ${GTEST_LIBRARIES} ${GTEST_MAIN})

add_executable(sample7_unittest unittest/sample7_unittest.cc)
target_link_libraries(sample7_unittest ${GTEST_LIBRARIES} ${GTEST_MAIN})

add_executable(sample8_unittest unittest/sample8_unittest.cc)
target_link_libraries(sample8_unittest ${GTEST_LIBRARIES} ${GTEST_MAIN})

add_executable(sample9_unittest unittest/sample9_unittest.cc)
target_link_libraries(sample9_unittest ${GTEST_LIBRARIES} )

add_executable(sample10_unittest unittest/sample10_unittest.cc)
target_link_libraries(sample10_unittest ${GTEST_LIBRARIES} )
