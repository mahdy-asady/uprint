#pragma once

#include <cstdio>


#define PLUGIN_NAME                 "uprint"

#define PLUGIN_VERSION              "1.0.0"

#define PLUGIN_WEBSITE              "https://mahdy-asady.github.io/uprint/"

#define INTERFACE_FN_NAME           "uprint"


#ifdef ENABLE_DEBUG
    #define DEBUG(fmt, ...) std::printf("[  uprint-debug  ]:\t" fmt "\n", ##__VA_ARGS__)
#else
    #define DEBUG(fmt, ...) ((void)0)
#endif
