// -*- mode:C++; tab-width:8; c-basic-offset:2; indent-tabs-mode:t -*-
// vim: ts=8 sw=2 smarttab
/*
 * Ceph distributed storage system
 *
 * Copyright (C) 2013,2014 Cloudwatt <libre.licensing@cloudwatt.com>
 * Copyright (C) 2014 Red Hat <contact@redhat.com>
 * Copyright (C) 2020 Feng Hualong <fenghualong1@126.com>
 *
 * Author: Loic Dachary <loic@dachary.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 */

#include "PluginRegistry.h"
//#include "ceph_ver.h"
//#include "common/ceph_context.h"
//#include "common/errno.h"
//#include "common/debug.h"
//#include "include/dlfcn_compat.h"
#include <dlfcn.h>
#include <iostream>
#include <errno.h>

#define PLUGIN_PREFIX "libceph_"
//#define SHARED_LIB_SUFFIX CMAKE_SHARED_LIBRARY_SUFFIX
#define SHARED_LIB_SUFFIX ".so"
#define PLUGIN_SUFFIX SHARED_LIB_SUFFIX
#define PLUGIN_INIT_FUNCTION "__ceph_plugin_init"
#define PLUGIN_VERSION_FUNCTION "__ceph_plugin_version"
#define PLUGIN_DIR "./lib/"

using std::map;
using std::string;


    PluginRegistry::PluginRegistry() :
            loading(false),
            disable_dlclose(false)
    {
    }

    PluginRegistry::~PluginRegistry()
    {
        if (disable_dlclose)
            return;

        //std::map<std::string,std::map<std::string, Plugin*> >::iterator
        for (auto i = plugins.begin(); i != plugins.end(); ++i) {
            //std::map<std::string,Plugin*>::iterator
            for (auto j = i->second.begin(); j != i->second.end(); ++j) {
                void *library = j->second->library;
                delete j->second;
                dlclose(library);
            }
        }
    }

    int PluginRegistry::remove(const std::string& type, const std::string& name)
    {
        //ceph_assert(ceph_mutex_is_locked(lock));

        //std::map<std::string,std::map<std::string,Plugin*> >::iterator
        auto i = plugins.find(type);
        if (i == plugins.end())
            return -ENOENT;
        std::map<std::string,Plugin*>::iterator j = i->second.find(name);
        if (j == i->second.end())
            return -ENOENT;

        std::cout << __func__ << " " << type << " " << name << std::endl;
        void *library = j->second->library;
        delete j->second;
        dlclose(library);
        i->second.erase(j);
        if (i->second.empty())
            plugins.erase(i);

        return 0;
    }

    int PluginRegistry::add(const std::string& type,
                            const std::string& name,
                            Plugin* plugin)
    {
        //ceph_assert(ceph_mutex_is_locked(lock));
        if (plugins.count(type) && plugins[type].count(name)) {
            return -EEXIST;
        }
        std::cout << __func__ << " " << type << " " << name
                      << " " << plugin << std::endl;
        plugins[type][name] = plugin;
        return 0;
    }

    Plugin *PluginRegistry::get_with_load(const std::string& type,
                                          const std::string& name)
    {
        std::lock_guard<std::mutex> l(lock);
        Plugin* ret = get(type, name);
        if (!ret) {
            int err = load(type, name);
            if (err == 0)
                ret = get(type, name);
        }
        return ret;
    }

    Plugin *PluginRegistry::get(const std::string& type,
                                const std::string& name)
    {
        //ceph_assert(ceph_mutex_is_locked(lock));
        Plugin *ret = 0;

        std::map<std::string,Plugin*>::iterator j;
        std::map<std::string,map<std::string,Plugin*> >::iterator i = plugins.find(type);
        if (i == plugins.end())
            goto out;
        j = i->second.find(name);
        if (j == i->second.end())
            goto out;
        ret = j->second;

        out:
        std::cout << __func__ << " " << type << " " << name
                      << " = " << ret << std::endl;
        return ret;
    }

    int PluginRegistry::load(const std::string &type,
                             const std::string &name)
    {
        //ceph_assert(ceph_mutex_is_locked(lock));
        std::cout << __func__ << " " << type << " " << name << std::endl;

        std::string fname = string() + PLUGIN_DIR + "/" + type + "/" + PLUGIN_PREFIX
                            + name + PLUGIN_SUFFIX;
        void *library = dlopen(fname.c_str(), RTLD_NOW);
        if (!library) {
            string err1(dlerror());
            // fall back to plugin_dir
            fname = string() + PLUGIN_DIR + "/" + PLUGIN_PREFIX +
                    name + PLUGIN_SUFFIX;
            library = dlopen(fname.c_str(), RTLD_NOW);
            if (!library) {
                std::cerr << __func__
                           << " failed dlopen(): \""	<< err1.c_str()
                           << "\" or \"" << dlerror() << "\""
                           << std::endl;
                return -EIO;
            }
        }

//        const char * (*code_version)() = (const char *(*)())dlsym(library, PLUGIN_VERSION_FUNCTION);
//        if (code_version == NULL) {
//            std::cerr << __func__ << " code_version == NULL" << dlerror() << std::endl;
//            return -EXDEV;
//        }
//        if (code_version() != string(CEPH_GIT_NICE_VER)) {
//            lderr(cct) << __func__ << " plugin " << fname << " version "
//                       << code_version() << " != expected "
//                       << CEPH_GIT_NICE_VER << dendl;
//            dlclose(library);
//            return -EXDEV;
//        }

        int (*code_init)(const std::string& type, const std::string& name) =
        (int (*)(const std::string& type, const std::string& name))dlsym(library, PLUGIN_INIT_FUNCTION);
        if (code_init) {
            int r = code_init(type, name);
            if (r != 0) {
                std::cerr << __func__ << " " << fname << " "
                           << PLUGIN_INIT_FUNCTION << "("
                           << "," << type << "," << name << "): " //<< cpp_strerror(r)
                           << std::endl;
                dlclose(library);
                return r;
            }
        } else {
            std::cerr << __func__ << " " << fname << " dlsym(" << PLUGIN_INIT_FUNCTION
                       << "): " << dlerror() << std::endl;
            dlclose(library);
            return -ENOENT;
        }

        Plugin *plugin = get(type, name);
        if (plugin == 0) {
            std::cerr << __func__ << " " << fname << " "
                       << PLUGIN_INIT_FUNCTION << "()"
                       << "did not register plugin type " << type << " name " << name
                       << std::endl;
            dlclose(library);
            return -EBADF;
        }

        plugin->library = library;

        std::cout << __func__ << ": " << type << " " << name
                      << " loaded and registered" << std::endl;
        return 0;
    }
