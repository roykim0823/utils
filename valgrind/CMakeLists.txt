cmake_minimum_required(VERSION 2.8)
project(valgrind)

set(CMAKE_C_FLAGS "-g -Wall")

file(GLOB SRCS "src/*.c")

foreach(src IN LISTS SRCS) 
#	message(STATUS "${src}")
	get_filename_component(filename ${src} NAME)
	string(REPLACE "." ";" temp ${filename})
	list(GET temp 0 target)
	add_executable(${target} ${src})
	list(APPEND TARGET_LIST ${target})
endforeach()
