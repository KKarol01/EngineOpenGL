#include "buffer.hpp"

#include <stdexcept>
#include <cassert>

#include <glad/glad.h>

#include "../../engine/engine.hpp"

namespace eng {

    GLBuffer::GLBuffer(uint32_t flags) {
        _flags = flags;
        glCreateBuffers(1, &_handle);
    }

    void GLBuffer::push_data(const void *data, size_t data_size) {
        if (_capacity < _size + data_size) { _resize(_size + data_size); }

        if ((_flags & GL_DYNAMIC_STORAGE_BIT) != GL_DYNAMIC_STORAGE_BIT) {
            GLuint temp_buffer;
            glCreateBuffers(1, &temp_buffer);
            glNamedBufferStorage(temp_buffer, data_size, data, 0);

            glCopyNamedBufferSubData(temp_buffer, _handle, 0, _size, data_size);
            glDeleteBuffers(1, &temp_buffer);
        } else {
            glNamedBufferSubData(_handle, _size, data_size, data);
        }

        _size += data_size;
    }

    void GLBuffer::clear_invalidate() {
        glInvalidateBufferData(_handle);
        _size = 0;
    }

    void GLBuffer::bind(uint32_t GL_TARGET) { glBindBuffer(GL_TARGET, handle()); }

    void GLBuffer::bind_base(uint32_t GL_TARGET, uint32_t base) {
        glBindBufferBase(GL_TARGET, base, handle());
    }

    void GLBuffer::_resize(size_t required_size) {
        size_t new_capacity = fmaxl(required_size * GROWTH_FACTOR, 1.);

        uint32_t old_handle = _handle;
        uint32_t new_handle;

        glCreateBuffers(1, &new_handle);
        glNamedBufferStorage(new_handle, new_capacity, 0, _flags);
        glCopyNamedBufferSubData(old_handle, new_handle, 0, 0, _size);
        glDeleteBuffers(1, &old_handle);

        _handle   = new_handle;
        _capacity = new_capacity;

        on_handle_change.emit(id);
    }

    GLBuffer::~GLBuffer() { glDeleteBuffers(1, &_handle); }

    GLVao::GLVao(std::initializer_list<GLVaoBinding> bindings,
                 std::initializer_list<GLVaoAttribute> attributes,
                 Handle<GLBuffer> ebo)
        : _bindings{bindings}, _attributes{attributes}, _ebo{ebo} {
        glCreateVertexArrays(1, &_handle);
        _calculate_attr_offsets_if_zeros();
        _configure_bindings();
        _configure_attributes();
    }

    GLVao::~GLVao() { glDeleteVertexArrays(1, &_handle); }

    void GLVao::bind() const { glBindVertexArray(_handle); }

    void GLVao::_calculate_attr_offsets_if_zeros() {
        bool only_zeros_as_offsets = true;

        for (const auto &a : _attributes) {
            if (a.byte_offset != 0u) {
                only_zeros_as_offsets = false;
                break;
            }
        }

        if (only_zeros_as_offsets == false) { return; }

        std::unordered_map<uint32_t, uint32_t> attr_offset;
        for (auto &a : _attributes) {
            uint32_t &offset = attr_offset[a.binding_id];
            a.byte_offset    = offset;
            offset += a.size * sizeof(float); // TODO: Probably need to support more byte sizes...
        }
    }

    void GLVao::_configure_bindings() {
        for (const auto &b : _bindings) {
            glVertexArrayVertexBuffer(
                _handle, b.binding_id, b.buffer_handle.id, b.offset, b.stride);
        }
    }

    void GLVao::_configure_attributes() {
        for (const auto &a : _attributes) {

            glVertexArrayAttribBinding(_handle, a.attr_id, a.binding_id);
            glVertexArrayAttribFormat(_handle,
                                      a.attr_id,
                                      a.size,
                                      _get_gl_attr_format(a.format),
                                      a.normalize,
                                      a.byte_offset);
            glEnableVertexArrayAttrib(_handle, a.attr_id);
        }
    }

    uint32_t GLVao::_get_gl_attr_format(ATTR_FORMAT format) {
        switch (format) {
        case eng::ATTR_FORMAT::FLOAT: return GL_FLOAT;
        default: assert(false && "unsupported format");
        }
    }

} // namespace eng