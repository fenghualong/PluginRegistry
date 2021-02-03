set(plugin_library
    add_plugin.cpp)

add_library(add_plugin SHARED ${plugin_library})

set_target_properties(add_plugin PROPERTIES
  OUTPUT_NAME ceph_add_plugin
  VERSION 1.0.0
  SOVERSION 1)

# install(TARGET add_plugin DESTINATION "lib")
# add_dependency()
