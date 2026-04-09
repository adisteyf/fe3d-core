#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fe-render-api.h"

#if defined(__linux__) || defined(__APPLE__)
#include <dlfcn.h>
#define FER_LOAD(x) { \
  char sym[100] = #x; \
  strcat(sym, postfix); \
  x = dlsym(feb.handle, sym); \
  const char *dlsym_error = dlerror(); \
  if (dlsym_error) { \
    printf("can't load symbol: %s\n", dlsym_error); \
    dlclose(feb.handle); \
    *status = -2; \
    return feb; \
  }}
#endif // __linux__ || __APPLE__

#ifdef __linux__
#define FER_LIBEXT ".so"
#endif // __linux__

#ifdef __APPLE__
#define FER_LIBEXT ".dylib"
#endif // __APPLE__

#ifdef _WIN32
#include <windows.h>
#define FER_LIBEXT ".dll"
#define FER_LOAD(x) { \
  char sym[100] = #x; \
  strcat(sym, postfix); \
  x = (typeof(x))GetProcAddress(feb.handle, sym) \
  if (!x) { \
    DWORD error = GetLastError(); \
    printf("can't load symbol: %lu\n", error); \
    FreeLibrary(hLib); \
    *status = -2; \
    return feb; \
}}
#endif // _WIN32

FeBackend fe_load_backend(char *path, int *status)
{
  FeBackend feb;
  char *postfix = strchr(path, '_');

  char path_ext[99];
  strcat(path_ext, path);
  strcat(path_ext, FER_LIBEXT);

#if defined(__linux__) || defined(__APPLE__)
  feb.handle = dlopen(path_ext, RTLD_LAZY);
#endif // UNIX
#ifdef _WIN32
  feb.handle = LoadLibrary(path_ext);
#endif
  if (!feb.handle) {
    printf("failed to load backend %s\n", path_ext);
    *status = -1;
    return feb;
  }

#if defined(__linux__) || defined(__APPLE__)
  dlerror();
#endif // UNIX
  FER_LOAD(fe_renderapi_version);

  return feb;
}

void fe_free_backend(FeBackend *feb)
{
  if (feb->handle) {
#if defined(__linux__) || defined(__APPLE__)
    dlclose(feb->handle);
#endif // __linux__ || __APPLE__
#ifdef _WIN32
    FreeLibrary(feb->handle);
#endif // _WIN32
    feb->handle = 0;
  }
}
