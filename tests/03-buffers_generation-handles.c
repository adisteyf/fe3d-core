#include <stdio.h>
#include <assert.h>
#include "../fe-api/render/fe-render-api.h"

int main() {
  FeRenderDesc render_desc = {
    .path = "./libfer-backend_debugblank",
    .out_fd = stdout
  };

  FeRender render = fe_render_create(&render_desc);
  float vertices[] = { 0.0f, 1.0f };
  FeBufferDesc buffer_desc = {
    .size = sizeof(vertices),
    .data = vertices,
    .usage = FE_VERTEX_BUFFER,
  };

  FeBuffer old_buf = fe_create_buffer(render, &buffer_desc);
  printf("old: i=%u gen=%u\n", fe_object_index(old_buf), fe_object_generation(old_buf));
  fe_free_buffer(render, old_buf);

  FeBuffer new_buf = fe_create_buffer(render, &buffer_desc);
  printf("new: i=%u gen=%u\n", fe_object_index(new_buf), fe_object_generation(new_buf));
  fe_free_buffer(render, new_buf);

  assert(fe_object_index(old_buf) == fe_object_index(new_buf));
  assert(fe_object_generation(old_buf) != fe_object_generation(new_buf));

  fe_render_free(render);
  return 0;
}
