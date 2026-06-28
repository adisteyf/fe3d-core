//#include "../fe-api/render/fe-render-api.c"
#include "../fe-api/render/fe-render-api.h"
//#include "dl-loader.h"
#include <stdio.h>

int main() {
  FeRenderDesc render_desc = {
    .path = "./libfer-backend_debugblank",
    .out_fd = stdout
  };

  FeRender render = fe_render_create(&render_desc);
  FeRenderAPI ferapi = fe_renderapi_version();

  printf("%s\n%s\nv%d.%d.%d\n",
      ferapi.api_name, ferapi.desc, ferapi.version.major, ferapi.version.minor, ferapi.version.patch);

  fe_render_free(render);
  return 0;
}
