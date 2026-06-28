#ifndef __FE_ENGINE
#define __FE_ENGINE
#include <stdint.h>
#include <stdlib.h>
#define FE_MAKE_VERSION(x,y,z) (FeVersion){x,y,z}

typedef void * FeEngine;

typedef struct {
  uint32_t minor;
  uint32_t major;
  uint32_t patch;
} FeVersion;

typedef struct {
  FeVersion version;
} FeEngineDesc;

typedef struct {
  uint32_t gen;
  int alive;
  void *object;
} NullObject;

typedef struct {
  NullObject *objects;
  ulong count,capacity,alive;
  struct {
    ulong *list,size,capacity;
  } free;
} FeObjectsInfo;

static inline uint64_t
fe_object_make(uint32_t ind, uint32_t gen) {
  return ((uint64_t)gen<<32)|ind;
}

static inline uint32_t
fe_object_index(uint64_t h) {
  return (uint32_t)(h&0xffffffff);
}

static inline uint32_t
fe_object_generation(uint64_t h) {
  return (uint32_t)(h>>32);
}

FeEngine fe_engine_create(FeEngineDesc *desc);
void fe_engine_free(FeEngine engine);

uint64_t fe_create_object(FeObjectsInfo *info);
void fe_free_object(FeObjectsInfo *info, uint64_t obj);

#endif // __FE_ENGINE
