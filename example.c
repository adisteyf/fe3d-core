#include "fe-render-api.h"

#define fe_pass(cmd, desc) \
	for (int _fe_pass__i = (fe_begin_pass(cmd, desc), 0); !_fe_pass__i; fe_end_pass(cmd), _fe_pass__i++)
int main()
{
	FeContext *ctx = fe_init(&(FeInitDesc) {
			.backend_path = "path/to/backend w/o extension",
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

	fe_shutdown(ctx);
	return 0;
}

