#include "buffer.hpp"

#include "../../engine/engine.hpp"
#include "../../renderer/renderer.hpp"

#include <stdexcept>
#include <cassert>

#include <glad/glad.h>

eng::GLBuffer::GLBuffer(GLBufferDescriptor desc) : descriptor{desc} { glCreateBuffers(1, &descriptor.handle); }

eng::GLBuffer::GLBuffer(void *data, uint32_t size_bytes, GLBufferDescriptor desc) : GLBuffer(desc) {
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
    descriptor.on_handle_change.emit(descriptor.handle);
}

eng::GLBuffer::~GLBuffer() { glDeleteBuffers(1, &descriptor.handle); }

eng::GLVao::GLVao() { glCreateVertexArrays(1, &handle); }

eng::GLVao::GLVao(GLVao &&other) noexcept { *this = std::move(other); }

eng::GLVao &eng::GLVao::operator=(GLVao &&other) noexcept {
    handle       = other.handle;
    other.handle = 0u;

    return *this;
}

eng::GLVao::~GLVao() { glDeleteVertexArrays(1, &handle); }

void eng::GLVao::bind() const { glBindVertexArray(handle); }

void eng::GLVao::configure_binding(uint32_t id, BufferID bufferid, size_t stride, size_t offset) {
    descriptor.buff_bindings[id] = bufferid;

    auto &buffer = Engine::instance().renderer_->buffers[bufferid];
    glVertexArrayVertexBuffer(handle, id, buffer.descriptor.handle, offset, stride);

    buffer.descriptor.on_handle_change.connect([this, handle = this->handle, id, offset, stride](uint32_t nh) {
        glVertexArrayVertexBuffer(handle, id, nh, offset, stride);
    });
}

void eng::GLVao::configure_ebo(BufferID bufferid) {

    descriptor.ebo_set    = true;
    descriptor.ebo_buffer = bufferid;

    auto &buffer = Engine::instance().renderer_->buffers[bufferid];
    glVertexArrayElementBuffer(handle, buffer.descriptor.handle);

    buffer.descriptor.on_handle_change.connect(
        [handle = this->handle](uint32_t nh) { glVertexArrayElementBuffer(handle, nh); });
}

void eng::GLVao::configure_attributes(const std::vector<GLVaoAttributeDescriptor> &attributes) {
    descriptor.attributes = attributes;
    configure_attributes();
}

void eng::GLVao::configure_attributes(uint32_t size,
                                      GL_FORMAT_ gl_format,
                                      uint32_t type_size_bytes,
                                      const std::vector<GLVaoAttributeSameFormat> &attributes) {
    std::unordered_map<uint32_t, uint32_t> binding_offset;
    for (const auto &a : attributes) {
        descriptor.attributes.emplace_back(a.idx, a.binding, size, binding_offset[a.binding], gl_format, a.normalize);
        binding_offset[a.binding] += type_size_bytes * size;
    }

    configure_attributes();
}

void eng::GLVao::configure_attributes(GL_FORMAT_ gl_format,
                                      uint32_t type_size_bytes,
                                      const std::vector<GLVaoAttributeSameType> &attributes) {
    std::unordered_map<uint32_t, uint32_t> binding_offset;
    for (const auto &a : attributes) {
        descriptor.attributes.emplace_back(a.idx, a.binding, a.size, binding_offset[a.binding], gl_format, a.normalize);
        binding_offset[a.binding] += type_size_bytes * a.size;
    }

    configure_attributes();
}

void eng::GLVao::configure_attributes() {
    for (const auto &a : descriptor.attributes) {
        glEnableVertexArrayAttrib(handle, a.idx);
        glVertexArrayAttribBinding(handle, a.idx, a.binding);

        switch (a.gl_format) {
        case GL_FORMAT_FLOAT: glVertexArrayAttribFormat(handle, a.idx, a.size, GL_FLOAT, a.normalize, a.offset); break;
        default: throw std::runtime_error{"Unrecognised gl_func"};
        }
    }
}

eng::GLVaoAttributeDescriptor::GLVaoAttributeDescriptor(uint32_t idx, uint32_t binding, uint32_t size, uint32_t offset)
    : idx{idx}, binding{binding}, size{size}, offset{offset} {}

eng::GLVaoAttributeDescriptor::GLVaoAttributeDescriptor(
    uint32_t idx, uint32_t binding, uint32_t size, uint32_t offset, GL_FORMAT_ gl_format, bool normalize)
    : GLVaoAttributeDescriptor(idx, binding, size, offset) {
    this->normalize = normalize;
    this->gl_format   = gl_format;
}
