#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>
#include "SDLerr_helper.h"


#define WIN_WIDTH 800
#define WIN_HEIGHT 600

static SDL_Window* s_win;
static SDL_GPUDevice* s_gpudev;
static SDL_GPUBuffer* s_vertbuf;
static SDL_GPUTransferBuffer* s_transferbuf;
static SDL_GPUGraphicsPipeline* s_graphics_pipeline;

volatile bool running = true;


struct vertex {
	float x, y, z;    // vec3 position
	float r, g, b, a; // vec4 color
};

static constexpr struct vertex tri_verts[3] = {
	{ 0.0f, 0.5f, 0.0f,    1.0f, 0.0f, 0.0f, 1.0f }, // top
	{ -0.5f, -0.5f, 0.0f,  1.0f, 1.0f, 0.0f, 1.0f }, // left
	{ 0.5f, -0.5f, 0.0f,   1.0f, 0.0f, 1.0f, 1.0f }, // right
};

static SDL_GPUShader* create_shader(SDL_GPUDevice* device, const char code[], size_t code_size, SDL_GPUShaderStage shader_stage,
	uint32_t sampler_cnt, uint32_t uniformbuf_cnt, uint32_t storagebuf_cnt, uint32_t storage_tex_Cnt)
{
	return SDL_CreateGPUShader(device,
		&(struct SDL_GPUShaderCreateInfo) {
			.code = (unsigned char*)code,
			.code_size = code_size,
			.entrypoint = "main",
			.format = SDL_GPU_SHADERFORMAT_SPIRV,
			.stage = shader_stage,
			.num_samplers = sampler_cnt,
			.num_uniform_buffers = uniformbuf_cnt,
			.num_storage_buffers = storagebuf_cnt,
			.num_storage_textures = storage_tex_Cnt
		}
	);
}

/* SOURCE : https://hamdy-elzanqali.medium.com/let-there-be-triangles-sdl-gpu-edition-bd82cf2ef615 */
int main()
{
	FAILON(SDL_Init(SDL_INIT_VIDEO));

	ASSIORFAIL(s_win, SDL_CreateWindow("SDL3's GPU API!!!", WIN_WIDTH, WIN_HEIGHT, SDL_WINDOW_EXTERNAL));
	ASSIORFAIL(s_gpudev, SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, true, nullptr));
	FAILON(SDL_ClaimWindowForGPUDevice(s_gpudev, s_win));

	const char vert_shader_code[] = {
		#embed "../build/shader/shader.vert.spv"
	};
	const char frag_shader_code[] = {
		#embed "../build/shader/shader.frag.spv"
	};
	DECLORFAIL(SDL_GPUShader*, vert_shader, create_shader(s_gpudev, vert_shader_code, sizeof(vert_shader_code), SDL_GPU_SHADERSTAGE_VERTEX, 0, 0, 0, 0));
	DECLORFAIL(SDL_GPUShader*, frag_shader, create_shader(s_gpudev, frag_shader_code, sizeof(frag_shader_code), SDL_GPU_SHADERSTAGE_FRAGMENT, 0, 0, 0, 0));
	ASSIORFAIL(s_graphics_pipeline, SDL_CreateGPUGraphicsPipeline(s_gpudev,
		&(struct SDL_GPUGraphicsPipelineCreateInfo) {
			.target_info = {
				.num_color_targets = 1,
				.color_target_descriptions = (struct SDL_GPUColorTargetDescription[1]) {{
					.format = SDL_GetGPUSwapchainTextureFormat(s_gpudev, s_win),
					.blend_state.enable_blend = true,
					.blend_state.color_blend_op = SDL_GPU_BLENDOP_ADD,
					.blend_state.alpha_blend_op = SDL_GPU_BLENDOP_ADD,
					.blend_state.src_color_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA,
					.blend_state.dst_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
					.blend_state.src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA,
					.blend_state.dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
				}},
			},
			.vertex_input_state.num_vertex_buffers = 1,
			.vertex_input_state.vertex_buffer_descriptions = (struct SDL_GPUVertexBufferDescription[1]) {{
				.slot = 0,
				.input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX,
				.instance_step_rate = 0,
				.pitch = sizeof(struct vertex),
			}},
			.vertex_input_state.num_vertex_attributes = 2,
			.vertex_input_state.vertex_attributes = (struct SDL_GPUVertexAttribute[2]) {
				{	// a_position
					.buffer_slot = 0, // fetch data from the buffer at slot 0
					.location = 0,    // layout (location = 0) in shader
					.format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3, //vec3
					.offset = 0,      // start from the first byte from current buffer position
				},
				{	// a_color
					.buffer_slot = 0, // use buffer at slot 0
					.location = 1,    // layout (location = 1) in shader
					.format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4, //vec4
					.offset = sizeof(float) * 3, // 4th float from current buffer position
				}
			},
			.rasterizer_state.fill_mode = SDL_GPU_FILLMODE_FILL,
			.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
			.vertex_shader = vert_shader,
			.fragment_shader = frag_shader,
		}
	));
	SDL_ReleaseGPUShader(s_gpudev, vert_shader);
	SDL_ReleaseGPUShader(s_gpudev, frag_shader);

	s_vertbuf = SDL_CreateGPUBuffer(s_gpudev, 
		&(struct SDL_GPUBufferCreateInfo) {
			.size = sizeof(tri_verts),
			.usage = SDL_GPU_BUFFERUSAGE_VERTEX,
		}
	);
	s_transferbuf = SDL_CreateGPUTransferBuffer(s_gpudev,
		&(struct SDL_GPUTransferBufferCreateInfo) {
			.size = sizeof(tri_verts),
			.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
		}
	);

	DECLORFAIL(struct vertex*, vert_data, SDL_MapGPUTransferBuffer(s_gpudev, s_transferbuf, false));
	// Copy triangle data into the transfer buffer to upload it to the GPU later on
	SDL_memcpy(vert_data, tri_verts, sizeof(tri_verts));
	SDL_UnmapGPUTransferBuffer(s_gpudev, s_transferbuf);

	DECLORFAIL(SDL_GPUCommandBuffer*, cmdbuf, SDL_AcquireGPUCommandBuffer(s_gpudev));
	SDL_GPUCopyPass* cpy_pass = SDL_BeginGPUCopyPass(cmdbuf);
	SDL_UploadToGPUBuffer(cpy_pass, 
		&(struct SDL_GPUTransferBufferLocation) {
			.transfer_buffer = s_transferbuf,
			.offset = 0, // start from the beginning
		},
		&(struct SDL_GPUBufferRegion) {
			.buffer = s_vertbuf,
			.size = sizeof(tri_verts),
			.offset = 0, // begin writing from the first vertex
		},
		true
	);
	SDL_EndGPUCopyPass(cpy_pass);
	FAILON(SDL_SubmitGPUCommandBuffer(cmdbuf));

	SDL_GPUTexture* swpchain_tex;
	uint32_t width, height;
	ASSIORFAIL(cmdbuf, SDL_AcquireGPUCommandBuffer(s_gpudev));
	FAILON(SDL_WaitAndAcquireGPUSwapchainTexture(cmdbuf, s_win, &swpchain_tex, &width, &height));

	SDL_GPURenderPass* render_pass = SDL_BeginGPURenderPass(cmdbuf,
		&(struct SDL_GPUColorTargetInfo) {
			.texture = swpchain_tex,
			.clear_color = { 0.0f, 0.0f, 0.0f, 1.0f },
			.load_op = SDL_GPU_LOADOP_CLEAR,
			.store_op = SDL_GPU_STOREOP_STORE,
		},
		1, nullptr
	);

	SDL_BindGPUGraphicsPipeline(render_pass, s_graphics_pipeline);
	SDL_BindGPUVertexBuffers(render_pass, 0, // bind one buffer starting from slot 0
		(struct SDL_GPUBufferBinding[1]) {{
			.buffer = s_vertbuf, // index 0 is slot 0 in this example
			.offset = 0, // start from the first byte
		}},
		1
	);
	SDL_DrawGPUPrimitives(render_pass, 3, 1, 0, 0);

	SDL_EndGPURenderPass(render_pass);
	FAILON(SDL_SubmitGPUCommandBuffer(cmdbuf));

//	--- Main Loop ---
	SDL_Event event;
	while (running) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_EVENT_QUIT) {
				running = false;
			}
		}
	}
//	End Main Loop ---

	SDL_ReleaseGPUBuffer(s_gpudev, s_vertbuf);
	SDL_ReleaseGPUTransferBuffer(s_gpudev, s_transferbuf);
	SDL_ReleaseGPUGraphicsPipeline(s_gpudev, s_graphics_pipeline);
	SDL_DestroyGPUDevice(s_gpudev);
	SDL_DestroyWindow(s_win);
}

