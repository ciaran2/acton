#pragma once

#ifdef __linux__
#ifndef _GNU_SOURCE
#define _GNU_SOURCE 1
#endif
#endif

#include <pthread.h>

#include "log.h"

#ifdef __gnu_linux__
    #define IS_GNU_LINUX
#elif  __APPLE__ && __MACH__
    #define IS_MACOS
#endif

int ioloop(void *);
void stop_ioloop();