#ifndef __FE_RENDER_API
#define __FE_RENDER_API

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "../../dl-loader/dl-loader.h"

typedef struct FeContext FeContext;
typedef uint32_t FeBuffer;
typedef uint32_t FePipeline;
typedef uint32_t FeShader;

#define FER_API_MAJOR 0
#define FER_API_MINOR 1
#define FER_API_PATCH 0

typedef struct {
  FeBackend feb;
} FeRInitDesc;

typedef struct {
  char desc[100];
  char api_name[15];
  int major,minor,patch;
} FeRenderAPI;

FeRenderAPI (*fe_renderapi_version)(void);
int fe_render_api(char *path, FeBackends *febs)
{
  int status;
  char *postfix = strchr(path, '_');
  febs->render = fe_load_backend(path, &status);

  fedl_sym syms[] = {
    FEDL_SYM(fe_renderapi_version)
  };

  fedl_loadsyms(&febs->render, syms, sizeof(syms)/sizeof(syms[0]), postfix);
  return status;
}

/**
 * @brief creates context
 * @return context
 * @see fe_shutdown
 */
FeContext *(*fe_render_init)(const FeRInitDesc *);

/**
 * @brief destroys context
 * @see fe_init
 */
void       (*fe_render_shutdown)(FeContext *);

typedef struct {
	size_t size;
	const void *data;
	uint32_t usage;
} FeBufferDesc;

/* create tier */
FeBuffer (*fe_create_buffer)(FeContext *, const FeBufferDesc *);

typedef struct {
	int stage;
	const char *code;
} FeShaderDesc;

FeShader (*fe_create_shader)(FeContext *, const FeShaderDesc *);

typedef struct {
	FeShader vs,fs;
} FePipelineDesc;

FePipeline (*fe_create_pipeline)(FeContext *, const FePipelineDesc *);

typedef enum {
	FE_CMD_BEGIN_PASS,
	FE_CMD_END_PASS,
	FE_CMD_BIND_PIPELINE,
	FE_CMD_BIND_VERTEX_BUFFER,
	FE_CMD_DRAW,
} FeCmdType;

typedef struct {
	float clear_color[4];
} FePassDesc;

typedef struct {
	FeCmdType type;
	union {
		FePassDesc pass;
		FePipeline pipeline;
		FeBuffer   buffer;
		struct { uint32_t vertex_count; } draw;
	};
} FeCmd;

typedef struct {
	FeCmd *data;
	uint32_t count;
	uint32_t capacity;
} FeCmdBuffer;

/* window tier */
/**
 * @brief is window closed
 * @return 1 if window was closed and 0 if it wasn't
 */
int (*fe_window_isclosed)(FeContext *);

/* API begin */
FeCmdBuffer *(*fe_cmd_begin)(FeContext *);
void         (*fe_cmd_end)(FeCmdBuffer *);

/**
 * @brief sumbits @ref FeCmdBuffer to GPU
 * submits command buffer to GPU & destroys @ref FeCmdBuffer
 */
void         (*fe_submit)(FeContext *, FeCmdBuffer *);

/* DSL */
void (*fe_begin_pass)(FeCmdBuffer *, const FePassDesc *);
void (*fe_end_pass)(FeCmdBuffer *);
void (*fe_bind_pipeline)(FeCmdBuffer *, FePipeline);
void (*fe_bind_vertex_buffer)(FeCmdBuffer *, FeBuffer);
void (*fe_draw)(FeCmdBuffer *, uint32_t vertex_count);

#define fe_pass(cmd, desc) \
	for (int _fe_pass__i = (fe_begin_pass(cmd, desc), 0); !_fe_pass__i; fe_end_pass(cmd), _fe_pass__i++)


#endif // __FE_RENDER_API
