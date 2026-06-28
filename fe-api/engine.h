#ifndef __FE_ENGINE
#define __FE_ENGINE
#include <stdint.h>
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

FeEngine fe_engine_create(FeEngineDesc *desc);
void fe_engine_free(FeEngine engine);

#endif // __FE_ENGINE
