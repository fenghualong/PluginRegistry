cmake_minimum_required(VERSION 3.15)
project(plugin_demo)

set(CMAKE_CXX_STANDARD 17)

FIND_LIBRARY(GNU_LIB dl)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
link_libraries(${BOOST_LIB} ${GNU_LIB})

add_executable(plugin_demo main.cpp PluginRegistry.cpp PluginRegistry.h test_add.cpp test_add.h)