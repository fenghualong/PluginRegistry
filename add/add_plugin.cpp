#include "../PluginRegistry.h"
#include "add_plugin.h"

int myAdd::add(int a, int b) { return a+b; };

int __ceph_plugin_init(const std::string& type, const std::string& name) {
    return plugin_registry->add(type, name, new addPlugin());
}
