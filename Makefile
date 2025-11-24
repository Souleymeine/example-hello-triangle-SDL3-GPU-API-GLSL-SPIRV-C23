override MAKEFLAGS += --no-builtin-rules

build_dir = ./build
src_dir   = ./src
shader_dir= ./src/shader

sources = $(wildcard $(src_dir)/*.c)
objects = $(sources:$(src_dir)/%.c=$(build_dir)/%.o)
shaders = $(wildcard $(shader_dir)/*)
spirv   = $(addsuffix .spv, $(addprefix $(build_dir)/shader/, $(notdir $(shaders))))

OPT    = false
CFLAGS = -g3 -DDEBUG
STDC   = c23
ifeq ($(OPT), true)
  CFLAGS = -O2 -DNDEBUG
endif

$(eval $(shell mkdir -p $(build_dir)/shader))

triangle: $(objects)
	gcc $^ -lSDL3 -o $@

build/%.o: $(src_dir)/%.c $(spirv)
	gcc -std=$(STDC) -c $< $(CFLAGS) -o $@ -I./include/

$(build_dir)/shader/%.spv: $(shader_dir)/%
	glslang $< -V -o $@

.PHONY:
clean:
	rm -rf $(build_dir) triangle
run: triangle
	./$<
shaders: $(spirv)

