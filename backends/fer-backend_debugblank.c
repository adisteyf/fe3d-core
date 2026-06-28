//#include "../fe-api/render/rhi.c"
#include "../fe-api/render/rhi.h"


FeRenderAPI fe_renderapi_version_debugblank(void)
{
  FeRenderAPI ferapi = { 0 };
  strcpy(ferapi.api_name, "DebugBlank");
  strcpy(ferapi.desc, "DebugBlank v0.0.0 test description");
  ferapi.version = FE_MAKE_VERSION(0, 1, 0);

  return ferapi;
}

static inline void
fe_cmd_create_buffer(FeRenderImpl *nctx, FeCmd *c)
{
  FeBuffer h = c->create_buffer.h;
  uint32_t ind = fe_object_index(h);

  NullObject *b = &nctx->objects[ind];
  const FeBufferDesc *desc = &c->create_buffer.desc;
  b->buffer.native.data = malloc(desc->size);

  memcpy(b->buffer.native.data, desc->data, desc->size);

  __FEDL_LOG(nctx->logfd, "create buffer handle=%zu size=%zu usage=%u\n", c->create_buffer.h, c->create_buffer.desc.size,
      c->create_buffer.desc.usage)
}

static inline void
fe_cmd_destroy_buffer(FeRenderImpl *nctx, FeCmd *c)
{
  FeBuffer h = c->create_buffer.h;
  uint32_t ind = fe_object_index(h);

  NullObject *b = &nctx->objects[ind];
  free(b->buffer.native.data);

  __FEDL_LOG(nctx->logfd, "destroy buffer handle=%zu\n", c->destroy_buffer.h)
}

void fe_execute_debugblank(FeRender ctx, FeCmd *c)
{
  FeRenderImpl *nctx = FE_GET_RENDER_IMPL(ctx);

  switch (c->type) {
case FE_CMD_CREATE_BUFFER:
{
  fe_cmd_create_buffer(nctx, c);
  break;
}

case FE_CMD_BIND_VERTEX_BUFFER:
{
  __FEDL_LOG(nctx->logfd, "bind buffer handle=%zu\n", c->bind_vertex_buffer.h)
  break;
}

case FE_CMD_DESTROY_BUFFER:
{
  fe_cmd_destroy_buffer(nctx, c);
  break;
}
  }
}

void fe_submit_debugblank(FeRender ctx, FeCmdBuffer *cmd)
{
  for (uint32_t i=0; i<cmd->count; ++i) {
    FeCmd *c = &cmd->data[i];
    fe_execute_debugblank(ctx, c);
  }
}
