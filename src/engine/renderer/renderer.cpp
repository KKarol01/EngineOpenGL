#include "renderer.hpp"

BufferID RE::create_buffer(GLBufferDescriptor desc) { return buffers.allocate(desc); }

VaoID RE::create_vao(GLVaoDescriptor desc) { return vaos.allocate(desc); }

GLBuffer &RE::get_buffer(BufferID id) { return buffers.get(id).type; }

GLVao &RE::get_vao(VaoID id) { return vaos.get(id).type; }
