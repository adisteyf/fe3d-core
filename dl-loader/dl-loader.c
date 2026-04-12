#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dl-loader.h"
#if defined(__linux__) || defined (__APPLE__)
#include <dlfcn.h>
#endif

int fedl_loadsyms(FeBackend *feb, fedl_sym *syms, ulong len, const char *postfix)
{
  for (ulong i=0;i<len;++i) {
    ulong symlen = strlen(syms[i].sym)+strlen(postfix)+1;
    char *sym = malloc(symlen); bzero(sym, symlen);
    strcat(sym, syms[i].sym);
    strcat(sym, postfix);
#if defined(__linux__) || defined(__APPLE__)
    *syms[i].fn = dlsym(feb->handle, sym);
#endif
#ifdef _WIN32
    *syms[i].fn = (typeof(syms[i].fn))GetProcAddress(feb.handle, sym);
#endif
    free(sym);

#if defined(__linux__) || defined(__APPLE__)
    const char *dlsym_error = dlerror();
    if (dlsym_error) {
      printf("can't load symbol: %s\n", dlsym_error);
      dlclose(feb->handle);
      return -1;
    }
#endif

#ifdef _WIN32
    if (!*syms[i].fn) {
      DWORD _error = GetLastError();
      printf("can't load symbol: %s\n", _error);
      FreeLibrary(feb->handle);
      return -1;
    }
#endif
  }

  return 0;
}

#ifdef __linux__
#define FER_LIBEXT ".so"
#endif // __linux__

#ifdef __APPLE__
#define FER_LIBEXT ".dylib"
#endif // __APPLE__

#ifdef _WIN32
#include <windows.h>
#define FER_LIBEXT ".dll"
#endif // _WIN32

FeBackend fe_load_backend(char *path, int *status)
{
  FeBackend feb;
  ulong path_extlen = strlen(path)+strlen(FER_LIBEXT)+1;

  char *path_ext = malloc(path_extlen); bzero(path_ext, path_extlen);
  strcat(path_ext, path);
  strcat(path_ext, FER_LIBEXT);

#if defined(__linux__) || defined(__APPLE__)
  feb.handle = dlopen(path_ext, RTLD_LAZY);
#endif // UNIX
#ifdef _WIN32
  feb.handle = LoadLibrary(path_ext);
#endif
  free(path_ext);
  if (!feb.handle) {
    printf("failed to load backend %s\n", path_ext);
    *status = -1;
    return feb;
  }

#if defined(__linux__) || defined(__APPLE__)
  dlerror();
#endif // UNIX
  //FER_LOAD(fe_renderapi_version);
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

void fe_free_backends(FeBackends *febs)
{
  fe_free_backend(&febs->render);
  fe_free_backend(&febs->physics);
  fe_free_backend(&febs->window);
  fe_free_backend(&febs->sound);
}

FeBackends fedl_init()
{
  FeBackends febs;
  febs.render.handle = 0;
  febs.physics.handle = 0;
  febs.sound.handle = 0;
  febs.window.handle = 0;
  return febs;
}

