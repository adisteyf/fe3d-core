#ifndef __FE_RENDER_RHI
#define __FE_RENDER_RHI
#include <stdint.h>
#include <stdlib.h>
#include "fe-render-api.h"
#define FE_GET_RENDER_IMPL(r) ((FeRenderImpl *)(r))

typedef struct {
  uint32_t gen;
  int alive;
  FeObjectType type;

  union {
    struct {
      uint32_t size;
      FeBufferUsage usage;

      struct {
        void *data;
      } native;
    } buffer;

    struct {
      void *code;
      uint32_t code_size;
      FeShaderCodeType type;
    } shader_module;
  };
} NullObject;

typedef struct {
  /* FeContext */
  void *logfd;

  /* API */
  struct {
    ulong count,capacity,alive;
    struct {
      ulong *list,size,capacity;
    } free;
  } objects_info;

  FeBackend backend;
  NullObject *objects;
  FeCmdBuffer *cmds;
  ulong framebuffer_count;
  ulong frame;
} FeRenderImpl;

#endif // __FE_RENDER_RHI
