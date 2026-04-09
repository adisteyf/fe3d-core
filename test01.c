#include "src/fe-render-api.h"
#include <stdio.h>

int main() {
  int st;
  FeBackend feb = fe_load_backend("./libfer-backend_debugblank", &st);
  if (fe_renderapi_version==0) {
    printf("null ptr\n");
    return 0;
  }
  FeRenderAPI ferapi = fe_renderapi_version();

  printf("%s\n%s\nv%d.%d.%d\n",
      ferapi.api_name, ferapi.desc, ferapi.major, ferapi.minor, ferapi.patch);

  fe_free_backend(&feb);
  return 0;
}
