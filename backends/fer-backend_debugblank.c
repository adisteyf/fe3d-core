#include "../fe-api/render/fe-render-api.c"


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


void fe_submit_debugblank(FeContext *ctx, FeCmdBuffer *cmd)
{
  for (uint32_t i=0; i<cmd->count; ++i) {
    FeCmd *c = &cmd->data[i];
    
    switch (c->type) {
case FE_CMD_CREATE_BUFFER:
{
  __FEDL_LOG(ctx->logfd, "create buffer handle=%zu size=%zu\n", c->create_buffer.h, c->create_buffer.desc.size)
  break;
}

case FE_CMD_BIND_VERTEX_BUFFER:
{
  __FEDL_LOG(ctx->logfd, "bind buffer handle=%zu\n", c->bind_vertex_buffer.h)
  break;
}

case FE_CMD_DESTROY_BUFFER:
{
  __FEDL_LOG(ctx->logfd, "destroy buffer handle=%zu\n", c->destroy_buffer.h)
  break;
}
    }
  }
}
