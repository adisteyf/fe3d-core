#include "src/fe-render-api.c"
#include <stdio.h>

int main()
{
  FeBackends febs = fedl_init();
  if (fe_render_api("./libfer-backend_debugblank", &febs)) {
    printf("failed to load fe_render backend\n");
    return -1;
  }

	FeContext *ctx = fe_render_init(&(FeRInitDesc) {
      .feb = febs.render,
	});

	float vertices[] = {
		0.0f, 0.5f,
	 -0.5f,-0.5f,
	  0.5f,-0.5f,
	};

	FeBuffer vbuf = fe_create_buffer(ctx, &(FeBufferDesc) {
		.size  = sizeof(vertices),
		.data  = vertices,
		.usage = 0,
	});

	FeShader vs = fe_create_shader(ctx, &(FeShaderDesc){
		.stage = 0,
		.code  = "vertex",
	});

	FeShader fs = fe_create_shader(ctx, &(FeShaderDesc){
		.stage = 1,
		.code  = "fragment",
	});

	FePipeline pipeline = fe_create_pipeline(ctx, &(FePipelineDesc) {
		.vs = vs,
		.fs = fs,
	});

	while (!fe_window_isclosed(ctx)) {
		FeCmdBuffer *cmd = fe_cmd_begin(ctx);
		FePassDesc pass = {
			.clear_color = {0.1f, 0.1f, 0.2f, 1.0f},
		};

		fe_pass(cmd, &pass) {
			fe_bind_pipeline(cmd, pipeline);
			fe_bind_vertex_buffer(cmd, vbuf);
			fe_draw(cmd, 3);
		}

		fe_cmd_end(cmd);
		fe_submit(ctx, cmd);
	}

	fe_render_shutdown(ctx);
  fe_free_backends(&febs);
	return 0;
}

