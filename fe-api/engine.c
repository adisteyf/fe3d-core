#include <stdlib.h>
#include <stdint.h>
#include "engine.h"

uint64_t fe_create_object(FeObjectsInfo *info)
{
  //FeRenderImpl *ctx = FE_GET_RENDER_IMPL(_ctx);

  /* allocator */
  if (info->count >= info->capacity) {
    info->capacity = (info->capacity) ? info->capacity * 2 : 8;
    info->objects = realloc(info->objects, info->capacity * sizeof(*info->objects));
  }

  ulong id;
  if (info->free.size > 0) {
    id = info->free.list[--info->free.size];
  } else {
    id = info->count++;
  }

  NullObject *b = &info->objects[id];
  b->gen++;
  uint64_t handle = fe_object_make(id, b->gen);


  b->alive = 1;
  info->alive++;

  return handle;
}

void fe_free_object(FeObjectsInfo *info, uint64_t obj)
{
  //FeRenderImpl *ctx = FE_GET_RENDER_IMPL(_ctx);
  NullObject *b = &info->objects[fe_object_index(obj)];
  free(b->object);

  /* allocator */
  if (info->free.size >= info->free.capacity) {
    info->free.capacity = (info->free.capacity) ? info->free.capacity * 2 : 4;
    info->free.list = realloc(info->free.list, info->free.capacity * sizeof(*info->free.list));
  }

  //__FEDL_LOG(logfd, "[INFO] free buffer id=%u\n", fe_object_index(obj))
  b->alive = 0;
  b->gen++;
  info->alive--;
  info->free.list[info->free.size++] = fe_object_index(obj);
}
