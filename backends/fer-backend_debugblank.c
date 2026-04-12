#include "../fe-api/render/fe-render-api.c"
#include <string.h>
#include "../dl-loader/dl-loader.h"
/* debugblank v0.1.0+feR0.1.0
 * debug backend for fe-render-api
 */

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

typedef struct {
  void *data;
  ulong size;
  int alive;
} NullBuffer;

typedef struct NullContext {
  /* FeContext */
  void *logfd;

  /* API */
  ulong buffer_count,buffer_capacity,buffers_alive;
  struct {
    ulong *list,size;
  } buffers_free;
  NullBuffer *buffers;

  ulong framebuffer_count;
  ulong frame;
} NullContext;
static NullContext *last_buf_context;

FeContext *fe_render_init_debugblank(const FeRInitDesc *desc) {
  NullContext *ctx = malloc(sizeof(NullContext));
  ctx->logfd = desc->out_fd;
  ctx->frame = 0;
  ctx->buffer_count = 0;
  ctx->buffer_capacity = 0;
  ctx->buffers_alive = 0;
  ctx->framebuffer_count = 0;
  __FEDL_LOG(ctx->logfd, "[INFO] DebugBlank initialised\n")
  return (FeContext*)ctx;
}

void fe_render_shutdown_debugblank(FeContext *_ctx)
{
  NullContext *ctx = (NullContext*)_ctx;

  if (ctx->buffers_alive != 0) {
    __FEDL_LOG(ctx->logfd, "[LEAK] %u buffers not destroyed", ctx->buffer_count)
  }

  for (ulong i=0; i<ctx->buffer_count; i++) {
    if (ctx->buffers[i].alive) {
      __FEDL_LOG(ctx->logfd, "[LEAK] buffer id=%u is alive\n", i)
      free(ctx->buffers[i].data);
    }
  }

  free(ctx->buffers);
  __FEDL_LOG(ctx->logfd, "[INFO] DebugBlank shutdown\n")
  free(ctx);
}

FeBuffer fe_create_buffer_debugblank(FeContext *_ctx, const FeBufferDesc *desc)
{
  NullContext *ctx = (NullContext*)_ctx;

  if (!desc || desc->size==0) {
    __FEDL_LOG(ctx->logfd, "[ERROR] invalid buffer desc\n")
    return 0;
  }

  /* allocator */
  if (ctx->buffer_count >= ctx->buffer_capacity) {
    ctx->buffer_capacity = (ctx->buffer_capacity) ? ctx->buffer_capacity * 2 : 8;
    ctx->buffers = realloc(ctx->buffers, ctx->buffer_capacity * sizeof(*ctx->buffers));
  }

  FeBuffer id = ctx->buffer_count++;
  ctx->buffers[id].size = desc->size;
  ctx->buffers[id].data = malloc(desc->size);

  if (desc->data) {
    memcpy(ctx->buffers[id].data, desc->data, desc->size);
  }

  ctx->buffers[id].alive = 1;
  ctx->buffers_alive++;
  __FEDL_LOG(ctx->logfd, "[INFO] create buffer id=%u size=%zu\n", id, desc->size)
  return id;
}

void fe_bind_vertex_buffer_debugblank(FeCmdBuffer *cmd, FeBuffer buf)
{
  if (buf >= last_buf_context->buffer_count) {
    __FEDL_LOG(last_buf_context->logfd, "[ERROR] invalid buffer id=%u\n", buf)
    return;
  }

  FeCmd c = {
    .type = FE_CMD_VERTEX_BUFFER,
  };
  c.buffer = buf;
  push_cmd(cmd, c);
}

void fe_free_buffer_debugblank(FeContext *_ctx, FeBuffer buf)
{
  NullContext *ctx = (NullContext*)_ctx;

  if (buf >= ctx->buffer_count) {
    __FEDL_LOG(ctx->logfd, "[ERROR] free invalid buffer id=%u\n", buf)
    return;
  }

  NullBuffer *b = &ctx->buffers[buf];
  if (!b->alive) {
    __FEDL_LOG(ctx->logfd, "[LEAK] double-free buffer id=%u\n", buf)
    return;
  }

  free(b->data);
  b->data = 0;
  b->size = 0;
  b->alive = 0;
  ctx->buffers_alive--;
  ctx->buffers_free.list[ctx->buffers_free.size++] = buf;

  __FEDL_LOG(ctx->logfd, "[INFO] free buffer id=%u\n", buf)
}