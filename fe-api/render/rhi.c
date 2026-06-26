#include "fe-render-api.c"
#include <string.h>
#include "../../dl-loader/dl-loader.h"

typedef struct {
  //void *data;
  uint32_t size,gen;
  int alive;

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

static void
push_cmd(FeCmdBuffer *cmd, FeCmd c)
{
  if (cmd->count >= cmd->capacity) {
    cmd->capacity = cmd->capacity ? cmd->capacity*2 : 64;
    cmd->data = realloc(cmd->data, sizeof(FeCmd)*cmd->capacity);
  }

  cmd->data[cmd->count++] = c;
}

FeCmdBuffer *
fe_cmd_begin(FeContext *ctx)
{
  FeCmdBuffer *cmd = calloc(1, sizeof(*cmd));
  cmd->ctx = ctx;
  return cmd;
}

void
fe_cmd_reset(FeContext *_ctx)
{
  NullContext *ctx = (void *)_ctx;
  bzero(ctx->cmds->data, ctx->cmds->count*sizeof(FeCmd));
  ctx->cmds->count = 0;
}

void
fe_cmd_submit(FeContext *_ctx)
{
  NullContext *ctx = (void *)_ctx;
  fe_submit(_ctx, ctx->cmds);
}


FeContext *fe_render_init(const FeRInitDesc *desc) {
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
  ctx->cmds = fe_cmd_begin((void *)ctx);
  ctx->cmds->ctx = (void*)ctx;
  __FEDL_LOG(ctx->logfd, "[INFO] DebugBlank initialised\n")
  return (FeContext*)ctx;
}

void fe_render_shutdown(FeContext *_ctx)
{
  NullContext *ctx = (NullContext*)_ctx;

  if (ctx->buffers_alive) {
    __FEDL_LOG(ctx->logfd, "[LEAK] %zu buffers not destroyed\n", ctx->buffer_count)
  }

  for (ulong i=0; i<ctx->buffer_count; i++) {
    if (ctx->buffers[i].alive) {
      __FEDL_LOG(ctx->logfd, "[LEAK] buffer id=%zu is alive\n", i)
      //free(ctx->buffers[i].data);
      //fe_free_buffer(ctx, ) TODO
    }
  }

  free(ctx->buffers);
  free(ctx->buffers_free.list);
  free(ctx->cmds->data);
  free(ctx->cmds);
  __FEDL_LOG(ctx->logfd, "[INFO] DebugBlank shutdown\n")
  free(ctx);
}

FeBuffer fe_create_buffer(FeContext *_ctx, const FeBufferDesc *desc)
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
  //b->data = malloc(desc->size);
  b->gen++;

  FeBuffer handle = fe_buffer_make(id, b->gen);

  /*if (desc->data) {
    memcpy(b->data, desc->data, desc->size);
  }*/

  b->alive = 1;
  ctx->buffers_alive++;
  __FEDL_LOG(ctx->logfd, "[INFO] create buffer cmd id=%zu size=%zu\n", id, desc->size)

  FeCmd c = {
    .type = FE_CMD_CREATE_BUFFER,
  };

  c.create_buffer.h = handle;
  c.create_buffer.desc = *desc;
  push_cmd(ctx->cmds, c);

  return handle;
}

int fe_check_buffer(FeContext *_ctx, FeBuffer h)
{
  NullContext *ctx = (void *)_ctx;
  if (fe_buffer_index(h) >= ctx->buffer_count) return 0;
  NullBuffer *b = &ctx->buffers[fe_buffer_index(h)];

  if (!b->alive) {
    __FEDL_LOG(ctx->logfd, "[ERROR] dead buffer id=%u\n", fe_buffer_index(h))
    return FE_DEAD_BUFFER;
  }

  if (b->gen != fe_buffer_generation(h)) {
    __FEDL_LOG(ctx->logfd, "[ERROR] dead buffer id=%u gen=%u\n", fe_buffer_index(h), fe_buffer_generation(h))
    return FE_STALE_BUFFER;
  }

  return FE_OK;
}

void fe_bind_vertex_buffer(FeCmdBuffer *cmd, FeBuffer buf)
{
  NullContext *ctx = (void*)cmd->ctx;

  if (fe_check_buffer((void*)ctx, buf) != FE_OK) {
    __FEDL_LOG(ctx->logfd, "[ERROR] invalid buffer id=%u\n", fe_buffer_index(buf))
    return;
  }

  FeCmd c = {
    .type = FE_CMD_BIND_VERTEX_BUFFER,
  };

  c.bind_vertex_buffer.h = buf;
  push_cmd(cmd, c);
}

void fe_free_buffer(FeContext *_ctx, FeBuffer buf)
{
  NullContext *ctx = (NullContext*)_ctx;

  if (fe_check_buffer(_ctx, buf) != FE_OK) {
    __FEDL_LOG(ctx->logfd, "[ERROR] free invalid buffer id=%u\n", fe_buffer_index(buf))
    return;
  }

  NullBuffer *b = &ctx->buffers[fe_buffer_index(buf)];

  /* allocator */
  if (ctx->buffers_free.size >= ctx->buffers_free.capacity) {
    ctx->buffers_free.capacity = (ctx->buffers_free.capacity) ? ctx->buffers_free.capacity * 2 : 4;
    ctx->buffers_free.list = realloc(ctx->buffers_free.list, ctx->buffers_free.capacity * sizeof(*ctx->buffers_free.list));
  }

  __FEDL_LOG(ctx->logfd, "[INFO] free buffer id=%u\n", fe_buffer_index(buf))
  //free(b->data);
  //b->data = 0;
  b->size = 0;
  b->alive = 0;
  b->gen++;
  ctx->buffers_alive--;
  ctx->buffers_free.list[ctx->buffers_free.size++] = fe_buffer_index(buf);

  FeCmd c = {
    .type = FE_CMD_DESTROY_BUFFER
  };

  c.destroy_buffer.h = buf;
  push_cmd(ctx->cmds, c);
}

