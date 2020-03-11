%module(directors="1") simCore

%feature("autodoc", "3");

%{
#include "simCore.h"
%}

#ifdef _WIN32
%include "windows.i"
#endif

// Include STL support
%include "std_map.i"
%include "std_string.i"
%include "std_vector.i"

typedef unsigned char uint8_t;
typedef signed char int8_t;
typedef unsigned short uint16_t;
typedef short int16_t;
typedef unsigned int uint32_t;
typedef int int32_t;

// Windows MSVC uses long long for 64 bit ints.  Linux g++ uses long
#ifdef MSVC
typedef unsigned long long uint64_t;
typedef long long int64_t;
#else
typedef unsigned long uint64_t;
typedef long int64_t;
#endif

#define SDKCORE_EXPORT


%include "simCoreCommon.i"
%include "simCoreCalc.i"
%include "simCoreEM.i"
%include "simCoreLUT.i"
%include "simCoreString.i"
%include "simCoreTime.i"
