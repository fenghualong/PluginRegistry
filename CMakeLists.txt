cmake_minimum_required(VERSION 3.10)
project(plugin_demo)

set(CMAKE_CXX_STANDARD 17)

FIND_LIBRARY(GNU_LIB dl)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
link_libraries(${BOOST_LIB} ${GNU_LIB})

include_directories(./ ./add)
add_subdirectory(add)

add_executable(plugin_demo main.cpp PluginRegistry.cpp PluginRegistry.h)
target_link_libraries(plugin_demo add_plugin)

# ADD_SUBDIRECTORY(lib)
# add_library(plugin_lib SHARED add/add_plugin.cpp)