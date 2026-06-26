//#include "../fe-api/render/fe-render-api.c"
#include "../fe-api/render/rhi.c"
#include "dl-loader.h"
#include <stdio.h>

int main() {
  FeBackends febs = {0};
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


  fe_render_shutdown(ctx);
  fe_free_backends(&febs);
  return 0;
}
