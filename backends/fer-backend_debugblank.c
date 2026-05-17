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
  ulong size,gen;
  int alive;
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

  ulong framebuffer_count;
  ulong frame;
} NullContext;
static NullContext *last_buf_context;

FeContext *fe_render_init_debugblank(const FeRInitDesc *desc) {
  NullContext *ctx = malloc(sizeof(NullContext));
  ctx->logfd = desc->out_fd;
  ctx->frame = 0;
  ctx->buffers = 0;
  ctx->buffer_count = 0;
  ctx->buffer_capacity = 0;
  ctx->buffers_free.list = 0;
  ctx->buffers_free.size = 0;
  ctx->buffers_free.capacity = 0;
  ctx->buffers_alive = 0;
  ctx->framebuffer_count = 0;
  __FEDL_LOG(ctx->logfd, "[INFO] DebugBlank initialised\n")
  return (FeContext*)ctx;
}

void fe_render_shutdown_debugblank(FeContext *_ctx)
{
  NullContext *ctx = (NullContext*)_ctx;

  if (ctx->buffers_alive != 0) {
    __FEDL_LOG(ctx->logfd, "[LEAK] %zu buffers not destroyed\n", ctx->buffer_count)
  }

  for (ulong i=0; i<ctx->buffer_count; i++) {
    if (ctx->buffers[i].alive) {
      __FEDL_LOG(ctx->logfd, "[LEAK] buffer id=%zu is alive\n", i)
      free(ctx->buffers[i].data);
    }
  }

  free(ctx->buffers);
  free(ctx->buffers_free.list);
  __FEDL_LOG(ctx->logfd, "[INFO] DebugBlank shutdown\n")
  free(ctx);
}

FeBuffer fe_create_buffer_debugblank(FeContext *_ctx, const FeBufferDesc *desc)
{
  NullContext *ctx = (NullContext*)_ctx;

  if (!desc || desc->size==0) {
    __FEDL_LOG(ctx->logfd, "[ERROR] invalid buffer desc\n")
    return FE_INVALID_BUFFER;
  }

  /* allocator */
  if (ctx->buffer_count >= ctx->buffer_capacity) {
    ctx->buffer_capacity = (ctx->buffer_capacity) ? ctx->buffer_capacity * 2 : 8;
    ctx->buffers = realloc(ctx->buffers, ctx->buffer_capacity * sizeof(*ctx->buffers));
  }

  ulong id;
  if (ctx->buffers_free.size > 0) {
    id = ctx->buffers_free.list[--ctx->buffers_free.size];
  } else {
    id = ctx->buffer_count++;
  }

  NullBuffer *b = &ctx->buffers[id];

  b->size = desc->size;
  b->data = malloc(desc->size);
  b->gen++;

  FeBuffer handle = {
    .ind = id,
    .gen = b->gen
  };

  if (desc->data) {
    memcpy(b->data, desc->data, desc->size);
  }

  b->alive = 1;
  ctx->buffers_alive++;
  __FEDL_LOG(ctx->logfd, "[INFO] create buffer id=%zu size=%zu addr=%p\n", id, desc->size, b->data)
  return handle;
}

inline int fe_check_buffer_debugblank(FeContext *_ctx, FeBuffer h)
{
  NullContext *ctx = (void *)_ctx;
  if (h.ind >= ctx->buffer_count) return 0;
  NullBuffer *b = &ctx->buffers[h.ind];

  if (b->alive && b->gen == h.gen) {
    if (b->alive) return FE_STALE_BUFFER;
    return FE_DEAD_BUFFER;
  }

  return FE_OK;
}

void fe_bind_vertex_buffer_debugblank(FeCmdBuffer *cmd, FeBuffer buf)
{
  NullContext *ctx = last_buf_context;

  if (fe_check_buffer_debugblank((void*)ctx, buf) != FE_OK) {
    __FEDL_LOG(last_buf_context->logfd, "[ERROR] invalid buffer id=%zu\n", buf.ind)
    return;
  }

  FeCmd c = {
    .type = FE_CMD_BIND_VERTEX_BUFFER,
    .buffer = buf,
  };

  //push_cmd(cmd, c);
}

void fe_free_buffer_debugblank(FeContext *_ctx, FeBuffer buf)
{
  NullContext *ctx = (NullContext*)_ctx;

  if (buf.ind >= ctx->buffer_count) {
    __FEDL_LOG(ctx->logfd, "[ERROR] free invalid buffer id=%zu\n", buf.ind)
    return;
  }

  NullBuffer *b = &ctx->buffers[buf.ind];
  if (!b->alive) {
    __FEDL_LOG(ctx->logfd, "[CORRUPT] double-free buffer id=%zu\n", buf.ind)
    return;
  }

  if (b->gen != buf.gen) {
    __FEDL_LOG(ctx->logfd, "[CORRUPT] stale buffer handle id=%zu gen=%zu\n", buf.ind, buf.gen)
  }

  /* allocator */
  if (ctx->buffers_free.size >= ctx->buffers_free.capacity) {
    ctx->buffers_free.capacity = (ctx->buffers_free.capacity) ? ctx->buffer_capacity * 2 : 4;
    ctx->buffers_free.list = realloc(ctx->buffers_free.list, ctx->buffers_free.capacity * sizeof(*ctx->buffers_free.list));
  }

  __FEDL_LOG(ctx->logfd, "[INFO] free buffer id=%zu addr=%p\n", buf.ind, b->data)
  free(b->data);
  b->data = 0;
  b->size = 0;
  b->alive = 0;
  b->gen++;
  ctx->buffers_alive--;
  ctx->buffers_free.list[ctx->buffers_free.size++] = buf.ind;
}

