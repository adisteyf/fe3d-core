#include "fe-render-api.h"
#include <string.h>
#include "../../dl-loader/dl-loader.h"
#include "rhi.h"

int fe_render_api(char *path, FeBackends *febs, void *outfd)
{
	int status=0;
	char *postfix = strchr(path, '_');
	febs->render = fe_load_backend(path, &status, outfd);
	if (status) return status;
	__FEDL_LOG(outfd, "[INFO] loading fe_render_api symbols\n")

	fedl_sym syms[] = {
		FEDL_SYM(fe_renderapi_version)
		// FEDL_SYM(fe_render_init)
		// FEDL_SYM(fe_render_shutdown)
		// FEDL_SYM(fe_create_buffer)
		// FEDL_SYM(fe_free_buffer)
		/*FEDL_SYM(fe_create_shader)
		FEDL_SYM(fe_create_pipeline)
		FEDL_SYM(fe_bind_pipeline)*/
		// FEDL_SYM(fe_bind_vertex_buffer)
		/*FEDL_SYM(fe_draw)
		FEDL_SYM(fe_cmd_begin)
		FEDL_SYM(fe_cmd_end)
    */
		FEDL_SYM(fe_submit)
		FEDL_SYM(fe_execute)
    /*
		FEDL_SYM(fe_free_backend)
		FEDL_SYM(fe_free_backends)*/
	};

	status = fedl_loadsyms(&febs->render, syms, sizeof(syms)/sizeof(syms[0]), postfix, outfd);
	return status;
}


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
  ctx->objects = 0;
  ctx->objects_info.count = 0;
  ctx->objects_info.capacity = 0;
  ctx->objects_info.free.list = 0;
  ctx->objects_info.free.size = 0;
  ctx->objects_info.free.capacity = 0;
  ctx->objects_info.alive = 0;
  ctx->framebuffer_count = 0;
  ctx->cmds = fe_cmd_begin((void *)ctx);
  ctx->cmds->ctx = (void*)ctx;
  __FEDL_LOG(ctx->logfd, "[INFO] DebugBlank initialised\n")
  return (FeContext*)ctx;
}

void fe_render_shutdown(FeContext *_ctx)
{
  NullContext *ctx = (NullContext*)_ctx;

  if (ctx->objects_info.alive) {
    __FEDL_LOG(ctx->logfd, "[LEAK] %zu buffers not destroyed\n", ctx->objects_info.count)
  }

  for (ulong i=0; i<ctx->objects_info.count; i++) {
    if (ctx->objects[i].alive) {
      __FEDL_LOG(ctx->logfd, "[LEAK] buffer id=%zu is alive\n", i)
      //free(ctx->buffers[i].data);
      //fe_free_buffer(ctx, ) TODO
    }
  }

  free(ctx->objects);
  free(ctx->objects_info.free.list);
  free(ctx->cmds->data);
  free(ctx->cmds);
  __FEDL_LOG(ctx->logfd, "[INFO] DebugBlank shutdown\n")
  free(ctx);
}

uint64_t fe_create_object(FeContext *_ctx)
{
  NullContext *ctx = (NullContext*)_ctx;

  /* allocator */
  if (ctx->objects_info.count >= ctx->objects_info.capacity) {
    ctx->objects_info.capacity = (ctx->objects_info.capacity) ? ctx->objects_info.capacity * 2 : 8;
    ctx->objects = realloc(ctx->objects, ctx->objects_info.capacity * sizeof(*ctx->objects));
  }

  ulong id;
  if (ctx->objects_info.free.size > 0) {
    id = ctx->objects_info.free.list[--ctx->objects_info.free.size];
  } else {
    id = ctx->objects_info.count++;
  }

  NullObject *b = &ctx->objects[id];
  b->gen++;
  uint64_t handle = fe_object_make(id, b->gen);


  b->alive = 1;
  ctx->objects_info.alive++;

  return handle;
}

FeBuffer fe_create_buffer(FeContext *_ctx, const FeBufferDesc *desc)
{
  NullContext *ctx = (void *)_ctx;
  if (!desc || desc->size==0) {
    __FEDL_LOG(ctx->logfd, "[ERROR] invalid buffer desc\n")
    return FE_INVALID_BUFFER;
  }

  FeBuffer h = fe_create_object(_ctx);
  uint32_t id = fe_object_index(h);
  NullObject *b = &ctx->objects[id];

  b->buffer.size = desc->size;
  b->buffer.usage = desc->usage;

  __FEDL_LOG(ctx->logfd, "[INFO] create buffer cmd id=%u size=%zu\n", id, desc->size)
  FeCmd c = {
    .type = FE_CMD_CREATE_BUFFER,
  };

  c.create_buffer.h = h;
  c.create_buffer.desc = *desc;
  fe_execute(_ctx, &c);

  return h;
}


int fe_check_buffer(FeContext *_ctx, FeBuffer h)
{
  NullContext *ctx = (void *)_ctx;
  if (fe_object_index(h) >= ctx->objects_info.count) return 0;
  NullObject *b = &ctx->objects[fe_object_index(h)];

  if (!b->alive) {
    __FEDL_LOG(ctx->logfd, "[ERROR] dead buffer id=%u\n", fe_object_index(h))
    return FE_DEAD_BUFFER;
  }

  if (b->gen != fe_object_generation(h)) {
    __FEDL_LOG(ctx->logfd, "[ERROR] dead buffer id=%u gen=%u\n", fe_object_index(h), fe_object_generation(h))
    return FE_STALE_BUFFER;
  }

  return FE_OK;
}

void fe_bind_vertex_buffer(FeCmdBuffer *cmd, FeBuffer buf)
{
  NullContext *ctx = (void*)cmd->ctx;

  if (fe_check_buffer((void*)ctx, buf) != FE_OK) {
    __FEDL_LOG(ctx->logfd, "[ERROR] invalid buffer id=%u\n", fe_object_index(buf))
    return;
  }

  FeCmd c = {
    .type = FE_CMD_BIND_VERTEX_BUFFER,
  };

  c.bind_vertex_buffer.h = buf;
  push_cmd(cmd, c);
}

void fe_free_object(FeContext *_ctx, uint64_t obj)
{
  NullContext *ctx = (NullContext*)_ctx;
  NullObject *b = &ctx->objects[fe_object_index(obj)];

  /* allocator */
  if (ctx->objects_info.free.size >= ctx->objects_info.free.capacity) {
    ctx->objects_info.free.capacity = (ctx->objects_info.free.capacity) ? ctx->objects_info.free.capacity * 2 : 4;
    ctx->objects_info.free.list = realloc(ctx->objects_info.free.list, ctx->objects_info.free.capacity * sizeof(*ctx->objects_info.free.list));
  }

  __FEDL_LOG(ctx->logfd, "[INFO] free buffer id=%u\n", fe_object_index(obj))
  b->alive = 0;
  b->gen++;
  ctx->objects_info.alive--;
  ctx->objects_info.free.list[ctx->objects_info.free.size++] = fe_object_index(obj);
}

void fe_free_buffer(FeContext *_ctx, FeBuffer buf)
{
  NullContext *ctx = (void *)_ctx;
  if (fe_check_buffer(_ctx, buf) != FE_OK) {
    __FEDL_LOG(ctx->logfd, "[ERROR] free invalid buffer id=%u\n", fe_object_index(buf))
    return;
  }

  fe_free_object(_ctx, buf);
  NullObject *b = &ctx->objects[fe_object_index(buf)];
  b->buffer.size = 0;

  FeCmd c = {
    .type = FE_CMD_DESTROY_BUFFER
  };

  c.destroy_buffer.h = buf;
  fe_execute(_ctx, &c);
}

/*FeShaderModule fe_create_shader_module(FeContext *_ctx, const FeShaderModuleDesc *desc)
{
  NullContext *ctx = (NullContext*)_ctx;
}*/
