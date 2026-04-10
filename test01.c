#include "src/fe-render-api.c"
#include <stdio.h>

int main() {
  FeBackends febs = fedl_init();
  if (fe_render_api("./libfer-backend_debugblank", &febs)) {
    printf("failed to load fe_render backend\n");
    return -1;
  }

  FeRenderAPI ferapi = fe_renderapi_version();

  printf("%s\n%s\nv%d.%d.%d\n",
      ferapi.api_name, ferapi.desc, ferapi.major, ferapi.minor, ferapi.patch);

  fe_free_backends(&febs);
  return 0;
}
