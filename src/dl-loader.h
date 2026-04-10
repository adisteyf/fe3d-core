#pragma once
#include <stdlib.h>
#define FEDL_SYM(x) { #x, (void**)&x },

typedef struct {
  char *sym;
  void **fn;
} fedl_sym;

typedef struct {
  char *absolute_path;
  void *handle;
} FeBackend;

typedef struct {
  FeBackend 
    render,
    sound,
    physics,
    window
  ;
} FeBackends;


FeBackends fedl_init(void);
int fedl_loadsyms(FeBackend *feb, fedl_sym *syms, ulong len, const char *postfix);
FeBackend fe_load_backend(char *path, int *status); /* w/o extension */
void      fe_free_backend(FeBackend *);
void      fe_free_backends(FeBackends *);

