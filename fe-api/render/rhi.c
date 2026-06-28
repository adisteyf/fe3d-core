#include "fe-render-api.h"
#include <stdlib.h>
#include <string.h>
#include "../../dl-loader/dl-loader.h"
#include "rhi.h"

/*int fe_render_api(char *path, FeBackends *febs, void *outfd)
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
		/ *FEDL_SYM(fe_create_shader)
		FEDL_SYM(fe_create_pipeline)
		FEDL_SYM(fe_bind_pipeline)*/
		// FEDL_SYM(fe_bind_vertex_buffer)
		/*FEDL_SYM(fe_draw)
		FEDL_SYM(fe_cmd_begin)
		FEDL_SYM(fe_cmd_end)
    * /
		FEDL_SYM(fe_submit)
		FEDL_SYM(fe_execute)
    / *
		FEDL_SYM(fe_free_backend)
		FEDL_SYM(fe_free_backends)* /
	};

	status = fedl_loadsyms(&febs->render, syms, sizeof(syms)/sizeof(syms[0]), postfix, outfd);
	return status;
}*/


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
fe_cmd_begin(FeRender ctx)
{
  FeCmdBuffer *cmd = calloc(1, sizeof(*cmd));
  cmd->render = ctx;
  return cmd;
}

void
fe_cmd_reset(FeRender _ctx)
{
  FeRenderImpl *ctx = FE_GET_RENDER_IMPL(_ctx);
  bzero(ctx->cmds->data, ctx->cmds->count*sizeof(FeCmd));
  ctx->cmds->count = 0;
}

void
fe_cmd_submit(FeRender _ctx)
{
  FeRenderImpl *ctx = FE_GET_RENDER_IMPL(_ctx);
  fe_submit(_ctx, ctx->cmds);
}


/*
FeRender fe_render_init(const FeRInitDesc *desc) {
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
}*/

FeRender fe_render_create(FeRenderDesc *desc)
{
  FeRenderImpl *impl = malloc(sizeof(FeRenderImpl));
  FeRender r = (FeRender)impl;

	int status=0;
	char *postfix = strchr(desc->path, '_');
	impl->backend = fe_load_backend(desc->path, &status, desc->out_fd);
	if (status) return status;
	__FEDL_LOG(desc->out_fd, "[INFO] loading fe_render_api symbols\n")

	fedl_sym syms[] = {
		FEDL_SYM(fe_renderapi_version)
		FEDL_SYM(fe_submit)
		FEDL_SYM(fe_execute)
	};

	status = fedl_loadsyms(&impl->backend, syms, sizeof(syms)/sizeof(syms[0]), postfix, desc->out_fd);
  if (status) {
    __FEDL_LOG(desc->out_fd, "[FATAL] failed to load fe_render backend")
    return 0;
  }

  impl->logfd = desc->out_fd;
  impl->frame = 0;
  impl->objects = 0;
  impl->objects_info.count = 0;
  impl->objects_info.capacity = 0;
  impl->objects_info.free.list = 0;
  impl->objects_info.free.size = 0;
  impl->objects_info.free.capacity = 0;
  impl->objects_info.alive = 0;
  impl->framebuffer_count = 0;
  impl->cmds = fe_cmd_begin(r);
  impl->cmds->render = r;
  __FEDL_LOG(impl->logfd, "[INFO] DebugBlank initialised\n")
  return r;
}

void fe_render_free(FeRender _ctx)
{
  FeRenderImpl *ctx = FE_GET_RENDER_IMPL(_ctx);

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

  fe_free_backend(&ctx->backend);
  free(ctx->objects);
  free(ctx->objects_info.free.list);
  free(ctx->cmds->data);
  free(ctx->cmds);
  __FEDL_LOG(ctx->logfd, "[INFO] DebugBlank shutdown\n")
  free(ctx);
}

uint64_t fe_create_object(FeRender _ctx)
{
  FeRenderImpl *ctx = FE_GET_RENDER_IMPL(_ctx);

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

FeBuffer fe_create_buffer(FeRender _ctx, const FeBufferDesc *desc)
{
  FeRenderImpl *ctx = FE_GET_RENDER_IMPL(_ctx);
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


int fe_check_buffer(FeRender _ctx, FeBuffer h)
{
  FeRenderImpl *ctx = FE_GET_RENDER_IMPL(_ctx);
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
  FeRenderImpl *ctx = FE_GET_RENDER_IMPL(cmd->render);

  if (fe_check_buffer(cmd->render, buf) != FE_OK) {
    __FEDL_LOG(ctx->logfd, "[ERROR] invalid buffer id=%u\n", fe_object_index(buf))
    return;
  }

  FeCmd c = {
    .type = FE_CMD_BIND_VERTEX_BUFFER,
  };

  c.bind_vertex_buffer.h = buf;
  push_cmd(cmd, c);
}

void fe_free_object(FeRender _ctx, uint64_t obj)
{
  FeRenderImpl *ctx = FE_GET_RENDER_IMPL(_ctx);
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

void fe_free_buffer(FeRender _ctx, FeBuffer buf)
{
  FeRenderImpl *ctx = FE_GET_RENDER_IMPL(_ctx);
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

FeShaderModule fe_create_shader_module(FeRender _ctx, const FeShaderModuleDesc *desc)
{
  FeRenderImpl *ctx = FE_GET_RENDER_IMPL(_ctx);
  if (!desc || desc->code==0) {
    __FEDL_LOG(ctx->logfd, "[ERROR] invalid shader module desc\n")
    return FE_INVALID_SHADER_MODULE;
  }

  FeBuffer h = fe_create_object(_ctx);
  uint32_t id = fe_object_index(h);
  NullObject *b = &ctx->objects[id];

  b->shader_module.type = desc->type;
  b->shader_module.code_size = desc->code_size;
  b->shader_module.code = malloc(b->shader_module.code_size+1);
  memcpy(b->shader_module.code, desc->code, desc->code_size);

  __FEDL_LOG(ctx->logfd, "[INFO] create shader cmd id=%u code_size=%u\n", id, desc->code_size)
  FeCmd c = {
    .type = FE_CMD_CREATE_SHADER_MODULE,
  };

  c.create_shader_module.h = h;
  c.create_shader_module.desc = *desc;
  fe_execute(_ctx, &c);

  return h;
}

void fe_free_shader_module(FeRender _ctx, FeShaderModule h)
{
  FeRenderImpl *ctx = FE_GET_RENDER_IMPL(_ctx);
  if (fe_check_buffer(_ctx, h) != FE_OK) {
    __FEDL_LOG(ctx->logfd, "[ERROR] free invalid buffer id=%u\n", fe_object_index(h))
    return;
  }

  fe_free_object(_ctx, h);
  NullObject *b = &ctx->objects[fe_object_index(h)];
  b->buffer.size = 0;

  FeCmd c = {
    .type = FE_CMD_DESTROY_SHADER_MODULE
  };

  c.destroy_shader_module.h = h;
  fe_execute(_ctx, &c);
}
