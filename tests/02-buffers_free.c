#ifndef __FE_RENDER_API
#include "../fe-api/render/fe-render-api.c"
#endif
#include <stdio.h>
#include <stdlib.h>

int main() {
  FeBackends febs = fedl_init();
  if (fe_render_api("./libfer-backend_debugblank", &febs, stdout)) {
    printf("failed to load fe_render backend\n");
    return -1;
  }

  FeRenderAPI ferapi = fe_renderapi_version();

  printf("%s\n%s\nv%d.%d.%d\n",
      ferapi.api_name, ferapi.desc, ferapi.major, ferapi.minor, ferapi.patch);
  FeRInitDesc fer_init_desc = {
    .out_fd = stdout,
    .feb = febs.render,
  };
  FeContext *ctx = fe_render_init(&fer_init_desc);

  char *_data = malloc(16);
  strcpy(_data, "hello world");
  FeBufferDesc buffer_desc = {
    .size = 16,
    .data = _data,
    .usage = 0,
  };
  FeBuffer buffer = fe_create_buffer(ctx, &buffer_desc);
  free(buffer_desc.data);
  fe_free_buffer(ctx, buffer);

  fe_render_shutdown(ctx);
  fe_free_backends(&febs);
  return 0;
}
