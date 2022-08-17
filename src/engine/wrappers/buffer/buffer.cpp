#include "buffer.hpp"
#include <stdexcept>

#include <glad/glad.h>

BUFFEROBJECT::BUFFEROBJECT(
    uint32_t id, void *data, uint32_t arr_length, uint32_t stride, uint32_t offset, uint32_t flags)
    : id{id}, offset{offset}, stride{stride} {
    glCreateBuffers(1, &handle);
    glNamedBufferStorage(handle, arr_length * stride, data, flags);
}

BUFFEROBJECT::BUFFEROBJECT(BUFFEROBJECT &&other) noexcept { *this = std::move(other); }

BUFFEROBJECT &BUFFEROBJECT::operator=(BUFFEROBJECT &&other) noexcept {
    handle       = other.handle;
    id           = other.id;
    offset       = other.offset;
    stride       = other.stride;
    other.handle = 0u;

    return *this;
}

VAO::VAO() : indice_count{0u} { glCreateVertexArrays(1, &handle); }

VAO::VAO(VAO &&other) noexcept { *this = std::move(other); }

VAO &VAO::operator=(VAO &&other) noexcept {
    handle       = other.handle;
    buffers      = std::move(other.buffers);
    indice_count = other.indice_count;
    draw         = std::move(other.draw);

    other.handle = 0u;

    return *this;
}

VAO::~VAO() {
    for (const auto &b : buffers) { glDeleteBuffers(1, &b.second.handle); }
    glDeleteVertexArrays(1, &handle);
}

void VAO::bind() const { glBindVertexArray(handle); }

void VAO::insert_vbo(BUFFEROBJECT &&buff) {
    glVertexArrayVertexBuffer(handle, buff.id, buff.handle, buff.offset, buff.stride);
    buffers.emplace(buff.id, std::move(buff));
}

void VAO::configure_binding(uint32_t id, uint32_t buffer_id, uint32_t stride, uint32_t offset) {
    if (!buffers.contains(buffer_id)) throw std::runtime_error{"Could not configure new binding point"};
    glVertexArrayVertexBuffer(handle, id, buffers.at(buffer_id).handle, offset, stride);
}

void VAO::insert_ebo(uint32_t indice_count, BUFFEROBJECT &&buff) {
    this->indice_count = indice_count;
    glVertexArrayElementBuffer(handle, buff.handle);
    ebo = std::move(buff);
}

void VAO::configure(std::initializer_list<ATTRCUSTORMFORMAT> formats) {

    for (auto &f : formats) { _configure_impl(f.format_func, f.id, f.buffer, f.size, f.type, f.normalized, f.offset); }
}

void VAO::configure(uint32_t size, uint32_t type, uint32_t type_size, std::initializer_list<ATTRSAMEFORMAT> formats) {
    std::map<uint32_t, uint32_t> buff_offsets;

    for (auto &f : formats) {
        _configure_impl(f.format_func, f.id, f.buffer, size, type, f.normalized, buff_offsets[f.buffer]);
        buff_offsets[f.buffer] += type_size * size;
    }
}

void VAO::configure(uint32_t type, uint32_t type_size, std::initializer_list<ATTRSAMETYPE> formats) {
    std::map<uint32_t, uint32_t> buff_offsets;

    for (auto &f : formats) {
        _configure_impl(f.format_func, f.id, f.buffer, f.size, type, f.normalized, buff_offsets[f.buffer]);
        buff_offsets[f.buffer] += type_size * f.size;
    }
}

void VAO::_configure_impl(ATTRFORMATCOMMON::GL_FUNC func,
                          uint32_t idx,
                          uint32_t buffer,
                          uint32_t size,
                          uint32_t type,
                          uint32_t normalized,
                          uint32_t offset) {

    glEnableVertexArrayAttrib(handle, idx);
    glVertexArrayAttribBinding(handle, idx, buffer);

    switch (func) {
    case ATTRFORMATCOMMON::GL_FUNC::FLOAT:
        glVertexArrayAttribFormat(handle, idx, size, type, normalized, offset);
        break;
    case ATTRFORMATCOMMON::GL_FUNC::INT: glVertexArrayAttribIFormat(handle, idx, size, type, offset); break;
    case ATTRFORMATCOMMON::GL_FUNC::LONG: glVertexArrayAttribLFormat(handle, idx, size, type, offset); break;
    default: break;
    }
}
