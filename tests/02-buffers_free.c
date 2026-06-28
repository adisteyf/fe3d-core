#include <stdio.h>
#include <stdlib.h>
#include "../fe-api/render/fe-render-api.h"

int main() {
  FeRenderDesc render_desc = {
    .path = "./libfer-backend_debugblank",
    .out_fd = stdout
  };

  FeRender render = fe_render_create(&render_desc);
  FeRenderAPI ferapi = fe_renderapi_version();

  char *_data = malloc(16);
  strcpy(_data, "hello world");
  FeBufferDesc buffer_desc = {
    .size = 16,
    .data = _data,
    .usage = FE_STORAGE_BUFFER,
  };
  FeBuffer buffer = fe_create_buffer(render, &buffer_desc);

  free(_data);
  fe_free_buffer(render, buffer);
  fe_render_free(render);
  return 0;
}
