#include "../fe-api/render/fe-render-api.c"
#include <stdio.h>
#include <stdlib.h>

int main()
{
  FeBackends febs = fedl_init();
  if (fe_render_api("./libfer-backend_debugblank", &febs, stdout)) {
    printf("failed to load fe_render backend\n");
    return -1;
  }

	FeRInitDesc init_desc = {
		.feb = febs.render,
  	.out_fd = stdout, /* stdout for logs */
  };
	FeContext *ctx = fe_render_init(&init_desc);

	float vertices[] = {
		0.0f, 0.5f,
	 -0.5f,-0.5f,
	  0.5f,-0.5f,
	};

	FeBufferDesc buf_desc = {
		.size  = sizeof(vertices),
		.data  = vertices,
		.usage = 0,
	};
	FeBuffer vbuf = fe_create_buffer(ctx, &buf_desc);

	FeShaderDesc shader_desc = {
		.stage = 0,
		.code  = "vertex",
	};
	FeShader vs = fe_create_shader(ctx, &shader_desc);

	FeShaderDesc shader_desc2 = {
		.stage = 1,
		.code  = "fragment",
	};
	FeShader fs = fe_create_shader(ctx, &shader_desc2);

	FePipelineDesc pipeline = {
		.vs = vs,
		.fs = fs,
	};
	FePipeline pipeline = fe_create_pipeline(ctx, &pipeline);

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

