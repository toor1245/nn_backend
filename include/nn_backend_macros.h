#ifndef NN_BACKEND_INCLUDE_MACROS_H
#define NN_BACKEND_INCLUDE_MACROS_H

////////////////////////////////////////////////////////////////////////////////
// OS
////////////////////////////////////////////////////////////////////////////////
#if (defined(__freebsd__) || defined(__FreeBSD__))
#define NN_BACKEND_OS_FREEBSD
#endif

#if defined(__ANDROID__)
#define NN_BACKEND_OS_ANDROID
#endif

#if defined(__linux__) && !defined(NN_BACKEND_OS_FREEBSD) && \
    !defined(NN_BACKEND_OS_ANDROID)
#define NN_BACKEND_OS_LINUX
#endif

#if (defined(_WIN64) || defined(_WIN32))
#define NN_BACKEND_OS_WINDOWS
#endif

#if (defined(__apple__) || defined(__APPLE__) || defined(__MACH__))
#include "TargetConditionals.h"
#if defined(TARGET_OS_OSX)
#define NN_BACKEND_OS_MACOS
#endif
#if defined(TARGET_OS_IPHONE)
// This is set for any non-Mac Apple products (IOS, TV, WATCH)
#define NN_BACKEND_OS_IPHONE
#endif
#endif

////////////////////////////////////////////////////////////////////////////////
// DllImport
////////////////////////////////////////////////////////////////////////////////
#if defined(NN_BACKEND__OS_WINDOWS)
#ifdef NN_BACKEND_DLL
#define NN_BACKEND_DLL_EXPORT __declspec(dllexport)
#else
#define NN_BACKEND_DLL_EXPORT __declspec(dllimport)
#endif
#else
#define NN_BACKEND_DLL_EXPORT
#endif // NN_BACKEND_OS_WINDOWS

#endif // NN_BACKEND_INCLUDE_MACROS_H
