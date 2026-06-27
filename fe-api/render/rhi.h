#ifndef __FE_RENDER_RHI
#define __FE_RENDER_RHI
#include <stdint.h>
#include <stdlib.h>
#include "fe-render-api.h"

typedef struct {
  //void *data;
  uint32_t size,gen;
  int alive;
  FeBufferUsage usage;

  struct {
    void *data;
  } native;
} NullBuffer;

typedef struct NullContext {
  /* FeContext */
  void *logfd;

  /* API */
  ulong buffer_count,buffer_capacity,buffers_alive;
  struct {
    ulong *list,size,capacity;
  } buffers_free;
  NullBuffer *buffers;

  FeCmdBuffer *cmds;
  ulong framebuffer_count;
  ulong frame;
} NullContext;

#endif // __FE_RENDER_RHI
