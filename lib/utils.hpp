#ifndef __COLPOSCOPY_UTILS
#define __COLPOSCOPY_UTILS

#include <string>
#include <iostream>
#include <sstream>

#ifdef WIN32
    #include <Windows.h>
#else
    #include <sys/time.h>
    #include <ctime>
#endif

#define __COLPOSCOPY_VERBOSE 0

typedef long long int64;
typedef unsigned long long uint64;

int num_chars(int n);

std::string spaced_d(int d, int n);

int64 GetTimeMs64();

#endif
