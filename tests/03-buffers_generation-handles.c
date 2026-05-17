#ifndef __FE_RENDER_API
#include "../fe-api/render/fe-render-api.c"
#endif
#include <stdio.h>
#include <assert.h>

int main() {
  FeBackends febs = fedl_init();
  if (fe_render_api("./libfer-backend_debugblank", &febs, stdout)) {
    printf("failed to load fe_render backend\n");
    return -1;
  }

  FeRInitDesc fer_init_desc = {
    .out_fd = stdout,
    .feb = febs.render,
  };
  FeContext *ctx = fe_render_init(&fer_init_desc);

  float vertices[] = { 0.0f, 1.0f };
  FeBufferDesc buffer_desc = {
    .size = sizeof(vertices),
    .data = vertices,
    .usage = 0,
  };

  FeBuffer old_buf = fe_create_buffer(ctx, &buffer_desc);
  printf("old: i=%zu gen=%zu\n", old_buf.ind, old_buf.gen);
  fe_free_buffer(ctx, old_buf);

  FeBuffer new_buf = fe_create_buffer(ctx, &buffer_desc);
  printf("new: i=%zu gen=%zu\n", new_buf.ind, new_buf.gen);
  fe_free_buffer(ctx, new_buf);

  assert(old_buf.ind == new_buf.ind);
  assert(old_buf.gen != new_buf.gen);


  fe_render_shutdown(ctx);
  fe_free_backends(&febs);
  return 0;
}
