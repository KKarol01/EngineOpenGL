#include "buffer_binder.hpp"

#include <glad/glad.h>
#include "pipeline.hpp"

void TargetBaseBufferBinder::bind() {
	glBindBufferBase(target, binding, RenderingPipeline::vvv); 
}