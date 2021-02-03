#include <iostream>
#include "PluginRegistry.h"
#include "add/add_plugin.h"

PluginRegistry *plugin_registry;

int main() {
    plugin_registry = new PluginRegistry();
    addPlugin* library = dynamic_cast<addPlugin*>(plugin_registry->get_with_load("test","add_plugin"));

    std::cout << (library == nullptr) << std::endl;

    myAdd* ma =  library->getMyAdd();

    std::cout << (ma == nullptr) << std::endl;

    std::cout << ma->add(1, 1) << std::endl;

    std::cout << "Hello, World!" << std::endl;
    return 0;
}
