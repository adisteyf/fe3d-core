#pragma once
#include <stdlib.h>
#include <stdio.h>
#define FEDL_SYM(x) { #x, (void**)&x },
#define __FEDL_LOG(x, y, ...) if (x) { fprintf((FILE *)x, y, ##__VA_ARGS__); }

typedef struct {
  char *sym;
  void **fn;
} fedl_sym;

typedef struct {
  char *absolute_path;
  void *handle;
} FeBackend;

typedef struct {
  union {
    FeBackend backends[4];
    FeBackend 
      render,
      sound,
      physics,
      window
    ;
  };
} FeBackends;


int fedl_loadsyms(FeBackend *feb, fedl_sym *syms, ulong len, const char *postfix, void *out);
FeBackend fe_load_backend(char *path, int *status, void *out); /* w/o extension */
void      fe_free_backend(FeBackend *);
void      fe_free_backends(FeBackends *);

