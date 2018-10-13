cmake_minimum_required(VERSION 2.8)
project(gtest_sample)

set(CMAKE_CXX_FLAGS "-g -Wall -Wextra -pthread")

# google test setup
set(GTEST_DIR "/home/roy/googletest/googletest")
include_directories (${GTEST_DIR}/include)
file(GLOB GTEST_LIB "${GTEST_DIR}/build/lib/*.a")

include_directories(include)
# sample1
add_executable(sample1_unittest "test/sample1_unittest.cc" "src/sample1.cc")
target_link_libraries(sample1_unittest ${GTEST_LIB})

#------------------------------------------------------------------------------
#file(GLOB SRCS "src/*.c")

#foreach(src IN LISTS SRCS) 
##	message(STATUS "${src}")
#	get_filename_component(filename ${src} NAME)
#	string(REPLACE "." ";" temp ${filename})
#	list(GET temp 0 target)
#	add_executable(${target} ${src})
#	list(APPEND TARGET_LIST ${target})
#endforeach()
