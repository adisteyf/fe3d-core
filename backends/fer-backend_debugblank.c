//#include "../fe-api/render/rhi.c"
#include "../fe-api/render/rhi.h"


FeRenderAPI fe_renderapi_version_debugblank(void)
{
  FeRenderAPI ferapi = { 0 };
  strcpy(ferapi.api_name, "DebugBlank");
  strcpy(ferapi.desc, "DebugBlank v0.0.0 test description");
  ferapi.major = 0;
  ferapi.minor = 1;
  ferapi.patch = 0;

  return ferapi;
}

void fe_execute_debugblank(FeContext *ctx, FeCmd *c)
{
  NullContext *nctx = (void *)ctx;

  switch (c->type) {
case FE_CMD_CREATE_BUFFER:
{
  FeBuffer h = c->create_buffer.h;
  uint32_t ind = fe_buffer_index(h);

  NullBuffer *b = &nctx->buffers[ind];
  const FeBufferDesc *desc = &c->create_buffer.desc;
  b->native.data = malloc(desc->size);

  memcpy(b->native.data, desc->data, desc->size);

  __FEDL_LOG(ctx->logfd, "create buffer handle=%zu size=%zu usage=%u\n", c->create_buffer.h, c->create_buffer.desc.size,
      c->create_buffer.desc.usage)
  break;
}

case FE_CMD_BIND_VERTEX_BUFFER:
{
  __FEDL_LOG(ctx->logfd, "bind buffer handle=%zu\n", c->bind_vertex_buffer.h)
  break;
}

case FE_CMD_DESTROY_BUFFER:
{
  FeBuffer h = c->create_buffer.h;
  uint32_t ind = fe_buffer_index(h);

  NullBuffer *b = &nctx->buffers[ind];
  free(b->native.data);

  __FEDL_LOG(ctx->logfd, "destroy buffer handle=%zu\n", c->destroy_buffer.h)
  break;
}
  }
}

void fe_submit_debugblank(FeContext *ctx, FeCmdBuffer *cmd)
{
  for (uint32_t i=0; i<cmd->count; ++i) {
    FeCmd *c = &cmd->data[i];
    fe_execute_debugblank(ctx, c);
  }
}
