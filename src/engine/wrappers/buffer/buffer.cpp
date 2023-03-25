#include "buffer.hpp"

#include "../../engine/engine.hpp"
#include "../../renderer/renderer.hpp"

#include <stdexcept>
#include <cassert>

#include <glad/glad.h>

eng::GLBuffer::GLBuffer(uint32_t flags) {
    descriptor.flags = flags;
    glCreateBuffers(1, &descriptor.handle);
}

eng::GLBuffer::GLBuffer(void *data, uint32_t size_bytes, uint32_t flags) : GLBuffer(flags) {
    push_data(data, size_bytes);
}

eng::GLBuffer::GLBuffer(GLBuffer &&other) noexcept { *this = std::move(other); }

eng::GLBuffer &eng::GLBuffer::operator=(GLBuffer &&other) noexcept {
    descriptor              = std::move(other.descriptor);
    other.descriptor.handle = 0;
    return *this;
}

void eng::GLBuffer::push_data(const void *data, size_t data_size) {
    if (descriptor.capacity < descriptor.size + data_size) { resize(descriptor.size + data_size); }

    if ((descriptor.flags & GL_DYNAMIC_STORAGE_BIT) != GL_DYNAMIC_STORAGE_BIT) {
        GLuint temp_buffer;
        glCreateBuffers(1, &temp_buffer);
        glNamedBufferStorage(temp_buffer, data_size, data, 0);

        glCopyNamedBufferSubData(temp_buffer, descriptor.handle, 0, descriptor.size, data_size);
        glDeleteBuffers(1, &temp_buffer);
    } else {
        glNamedBufferSubData(descriptor.handle, descriptor.size, data_size, data);
    }

    descriptor.size += data_size;
    on_handle_change.emit(descriptor);
}

void eng::GLBuffer::resize(size_t required_size) {
    size_t new_capacity = fmaxl(required_size * GROWTH_FACTOR, 1.);

    uint32_t old_handle = descriptor.handle;
    uint32_t new_handle;

    glCreateBuffers(1, &new_handle);
    glNamedBufferStorage(new_handle, new_capacity, 0, descriptor.flags);
    glCopyNamedBufferSubData(old_handle, new_handle, 0, 0, descriptor.size);
    glDeleteBuffers(1, &old_handle);

    descriptor.handle   = new_handle;
    descriptor.capacity = new_capacity;
}

eng::GLBuffer::~GLBuffer() { glDeleteBuffers(1, &descriptor.handle); }

eng::GLVao::GLVao() { glCreateVertexArrays(1, &descriptor.handle); }

eng::GLVao::GLVao(GLVao &&other) noexcept { *this = std::move(other); }

eng::GLVao &eng::GLVao::operator=(GLVao &&other) noexcept {
    descriptor = std::move(other.descriptor);

    other.descriptor.handle = 0u;

    return *this;
}

eng::GLVao::~GLVao() { glDeleteVertexArrays(1, &descriptor.handle); }

void eng::GLVao::bind() const { glBindVertexArray(descriptor.handle); }

void eng::GLVao::configure_binding(uint32_t id, BufferID bufferid, size_t stride, size_t offset) {
    descriptor.buff_bindings.push_back(GLVaoBufferBinding{id, bufferid});

    auto &buffer = Engine::instance().renderer_->get_buffer(bufferid);
    glVertexArrayVertexBuffer(descriptor.handle, id, buffer.descriptor.handle, offset, stride);

    buffer.on_handle_change.connect([id, offset, stride, handle = descriptor.handle](const auto &buff_desc) {
        glVertexArrayVertexBuffer(handle, id, buff_desc.handle, offset, stride);
    });
}

void eng::GLVao::configure_ebo(BufferID bufferid) {
    descriptor.ebo_buffer_id = bufferid;

    auto &buffer = Engine::instance().renderer_->get_buffer(bufferid);
    glVertexArrayElementBuffer(descriptor.handle, buffer.descriptor.handle);

    buffer.on_handle_change.connect(
        [handle = descriptor.handle](const auto &buff_desc) { glVertexArrayElementBuffer(handle, buff_desc.handle); });
}

void eng::GLVao::configure_attribute(GL_ATTR_ attribute_idx,
                                      uint32_t binding_id,
                                      uint32_t size,
                                      uint32_t byte_offset,
                                      GL_FORMAT_ format,
                                      bool normalize) {
    descriptor.attributes.emplace_back(attribute_idx, binding_id, size, byte_offset, format, normalize);
    glEnableVertexArrayAttrib(descriptor.handle, static_cast<uint32_t>(attribute_idx));
    glVertexArrayAttribBinding(descriptor.handle, attribute_idx, binding_id);

    switch (format) {
    case eng::GL_FORMAT_FLOAT:
        glVertexArrayAttribFormat(descriptor.handle, (uint32_t)attribute_idx, size, GL_FLOAT, normalize, byte_offset);
        break;
    default: assert(("unhandled format" && false));
    }
}

eng::GLVaoAttributeDescriptor::GLVaoAttributeDescriptor(GL_ATTR_ idx, uint32_t binding, uint32_t size, uint32_t offset)
    : idx{idx}, binding{binding}, size{size}, offset{offset} {}

eng::GLVaoAttributeDescriptor::GLVaoAttributeDescriptor(
    GL_ATTR_ idx, uint32_t binding, uint32_t size, uint32_t offset, GL_FORMAT_ gl_format, bool normalize)
    : GLVaoAttributeDescriptor(idx, binding, size, offset) {
    this->normalize = normalize;
    this->gl_format = gl_format;
}
