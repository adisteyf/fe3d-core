#ifndef __FE_RENDER_RHI
#define __FE_RENDER_RHI
#include <stdint.h>
#include <stdlib.h>
#include "fe-render-api.h"

typedef struct {
  uint32_t gen;
  int alive;
  FeObjectType type;

  // TODO: union
  struct {
    uint32_t size;
    FeBufferUsage usage;

    struct {
      void *data;
    } native;
  } buffer;
} NullObject;

typedef struct NullContext {
  /* FeContext */
  void *logfd;

  /* API */
  struct {
    ulong count,capacity,alive;
    struct {
      ulong *list,size,capacity;
    } free;
  } objects_info;

  NullObject *objects;
  FeCmdBuffer *cmds;
  ulong framebuffer_count;
  ulong frame;
} NullContext;

#endif // __FE_RENDER_RHI
