#ifndef __FE_RENDER_API
#define __FE_RENDER_API

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "../../dl-loader/dl-loader.h"

#define FE_INVALID_BUFFER 0x00
#define FE_INVALID_INDEX (unsigned long)-1
#define FE_STALE_BUFFER 0x03
#define FE_DEAD_BUFFER 0x04
#define FE_OK 0x00

typedef struct FeContext {
	void *logfd;
} FeContext;

typedef uint64_t FeBuffer;

static inline FeBuffer
fe_buffer_make(uint32_t ind, uint32_t gen) {
  return ((uint64_t)gen<<32)|ind;
}

static inline uint32_t
fe_buffer_index(FeBuffer h) {
  return (uint32_t)(h&0xffffffff);
}

static inline uint32_t
fe_buffer_generation(FeBuffer h) {
  return (uint32_t)(h>>32);
}


typedef uint32_t FePipeline;

#define FER_API_MAJOR 0
#define FER_API_MINOR 1
#define FER_API_PATCH 0

typedef struct {
  FeBackend feb;
	void *out_fd;
} FeRInitDesc;

typedef struct {
  char desc[100];
  char api_name[15];
  int major,minor,patch;
} FeRenderAPI;


int fe_render_api(char *path, FeBackends *febs, void *outfd);
FeRenderAPI (*fe_renderapi_version)(void);

/**
 * @brief creates context
 * @return context
 * @see fe_shutdown
 */
FeContext *fe_render_init(const FeRInitDesc *);

/**
 * @brief destroys context
 * @see fe_init
 */
void       fe_render_shutdown(FeContext *);

typedef enum {
  FE_VERTEX_BUFFER,
  FE_STORAGE_BUFFER,
} FeBufferUsage;

typedef struct {
	size_t size;
	const void *data;
	FeBufferUsage usage;
} FeBufferDesc;

/* create tier */
FeBuffer fe_create_buffer(FeContext *, const FeBufferDesc *);
void fe_free_buffer(FeContext *, FeBuffer);

typedef enum {
  FE_SHADER_CODE_TYPE_GLSL,
  FE_SHADER_CODE_TYPE_SPIRV,
} FeShaderCodeType;
typedef uint64_t FeShaderModule;

typedef struct {
  FeShaderModule module;
  char entry[32];
} FeShaderStageDesc;

typedef struct {
	void *code;
  FeShaderCodeType type;
} FeShaderModuleDesc;
FeShaderModule fe_create_shader_module(FeContext *, const FeShaderModuleDesc *);

typedef struct {
	FeShaderStageDesc vs,fs;
} FePipelineDesc;

FePipeline fe_create_pipeline(FeContext *, const FePipelineDesc *);

typedef enum {
	FE_CMD_BEGIN_PASS,
	FE_CMD_END_PASS,
	FE_CMD_BIND_PIPELINE,

  FE_CMD_CREATE_BUFFER,
  FE_CMD_DESTROY_BUFFER,

	FE_CMD_BIND_VERTEX_BUFFER,
	FE_CMD_DRAW,
} FeCmdType;

typedef struct {
	float clear_color[4];
} FePassDesc;

typedef struct {
	FeCmdType type;
	union {
    struct {
      FeBuffer h;
      FeBufferDesc desc;
    } create_buffer;

    struct {
      FeBuffer h;
    } destroy_buffer;

    struct {
      FeBuffer h;
    } bind_vertex_buffer;

    struct {
      uint32_t vertex_count;
    } draw;
	};
} FeCmd;

typedef struct {
	FeCmd *data;
  FeContext *ctx;
	uint32_t count;
	uint32_t capacity;
} FeCmdBuffer;

/* window tier */
/**
 * @brief is window closed
 * @return 1 if window was closed and 0 if it wasn't
 */
int fe_window_isclosed(FeContext *);

/* API begin */
FeCmdBuffer *fe_cmd_begin(FeContext *);
void         fe_cmd_end(FeCmdBuffer *);

/**
 * @brief sumbits @ref FeCmdBuffer to GPU
 * submits command buffer to GPU & destroys @ref FeCmdBuffer
 */
void         (*fe_submit)(FeContext *, FeCmdBuffer *);
void         (*fe_execute)(FeContext *, FeCmd *);

/* DSL */
void fe_begin_pass(FeCmdBuffer *, const FePassDesc *);
void fe_end_pass(FeCmdBuffer *);
void fe_bind_pipeline(FeCmdBuffer *, FePipeline);
void fe_bind_vertex_buffer(FeCmdBuffer *, FeBuffer);
void fe_draw(FeCmdBuffer *, uint32_t vertex_count);

#define fe_pass(cmd, desc) \
	for (int _fe_pass__i = (fe_begin_pass(cmd, desc), 0); !_fe_pass__i; fe_end_pass(cmd), _fe_pass__i++)

#endif // __FE_RENDER_API
