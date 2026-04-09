#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include "fe-render-api.h"

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

FeBackend fe_load_backend(char *path, int *status)
{
  FeBackend feb;
  char *postfix = strchr(path, '_');

  char path_ext[strlen(path)+strlen(".so\0")];
  strcat(path_ext, path);
  strcat(path_ext, ".so");

  feb.handle = dlopen(path_ext, RTLD_LAZY);
  if (!feb.handle) {
    printf("failed to load backend %s\n", path_ext);
    *status = -1;
    return feb;
  }

  dlerror();
  FER_LOAD(fe_renderapi_version);

  return feb;
}

void fe_free_backend(FeBackend *feb)
{
	if (feb->handle) {
		dlclose(feb->handle);
    feb->handle = 0;
	}
}
