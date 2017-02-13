#ifndef CSDKDEFS_H
#define CSDKDEFS_H
#if defined(_MSC_VER)
#   define SDK_EXPORT __declspec(dllexport)
#   define SDK_IMPORT __declspec(dllimport)
#elif defined(__GNUC__)
#   define SDK_EXPORT __attribute__((visibility("default")))
#   define SDK_IMPORT
#   if __GNUC__ > 4
#       define SDK_LOCAL __attribute__((visibility("hidden")))
#   else
#       define SDK_LOCAL
#   endif
#else
#   error("Don't know how to export shared object libraries")
#endif

#ifdef SDK_LIBRARY
#   define SDKSHARED_EXPORT SDK_EXPORT
#else
#   define SDKSHARED_EXPORT SDK_IMPORT
#endif
#endif // CSDKDEFS_H
