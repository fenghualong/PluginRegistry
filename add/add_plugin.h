#ifndef ADD_PLUGIN_H
#define ADD_PLUGIN_H

#include "../PluginRegistry.h"

extern PluginRegistry *plugin_registry;

class myAdd {
public:
    myAdd() {}
    ~myAdd() {}

    int add(int a, int b);

};

class addPlugin : public Plugin {
public:
    explicit addPlugin() {}
    ~addPlugin() {}

    myAdd* getMyAdd() { return new myAdd();}

};



#endif