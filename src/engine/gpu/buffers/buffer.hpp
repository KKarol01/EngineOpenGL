#pragma once

#include <cstdint>
#include <vector>
#include <initializer_list>
#include <cstdint>
#include <functional>
#include <map>
#include <variant>
#include <cassert>

#include <engine/types/types.hpp>
#include <engine/types/idresource.hpp>

namespace eng {
    struct GLBuffer : public IdResource<GLBuffer> {
        explicit GLBuffer() = default;
        explicit GLBuffer(uint32_t flags);
        GLBuffer(GLBuffer &&b) noexcept {
            id               = b.id;
            _handle          = b._handle;
            _flags           = b._flags;
            _size            = b._size;
            _capacity        = b._capacity;
            on_handle_change = std::move(b.on_handle_change);

            b.id      = 0;
            b._handle = 0;
        }
        ~GLBuffer();

        void push_data(const void *data, size_t size_bytes);
        void clear_invalidate();
        void bind(uint32_t GL_TARGET);
        void bind_base(uint32_t GL_TARGET, uint32_t base);

        uint32_t handle() const { return _handle; }
        size_t size() const { return _size; }
        size_t capacity() const { return _capacity; }

        Signal<uint32_t> on_handle_change;

      private:
        void _resize(size_t required_size);

        uint32_t _handle{0}, _flags{0};
        size_t _size{0}, _capacity{0};

        static constexpr float GROWTH_FACTOR{1.61f};
    };

    struct GLVaoBinding {
        explicit GLVaoBinding(uint32_t binding_id,
                              Handle<GLBuffer> buffer_handle,
                              size_t stride,
                              size_t offset)
            : binding_id{binding_id}, buffer_handle{buffer_handle}, stride{stride}, offset{offset} {
        }

        uint32_t binding_id{0u};
        Handle<GLBuffer> buffer_handle;
        size_t stride{0u}, offset{0u};
    };

    enum class ATTR_FORMAT { FLOAT };

    struct GLVaoAttribute {
        explicit GLVaoAttribute(uint32_t attr_id,
                                uint32_t binding_id,
                                uint32_t size,
                                uint32_t byte_offset,
                                ATTR_FORMAT format = ATTR_FORMAT::FLOAT,
                                bool normalize     = false)
            : attr_id{attr_id}, binding_id{binding_id}, size{size},
              byte_offset{byte_offset}, format{format}, normalize{normalize} {}

        uint32_t attr_id, binding_id, size, byte_offset;
        ATTR_FORMAT format = ATTR_FORMAT::FLOAT;
        bool normalize     = false;
    };

    class GLVao : public IdResource<GLVao> {
      public:
        explicit GLVao() = default;
        explicit GLVao(std::initializer_list<GLVaoBinding> bindings,
                       std::initializer_list<GLVaoAttribute> attributes,
                       Handle<GLBuffer> ebo = Handle<GLBuffer>{0});
        GLVao(GLVao &&other) noexcept {
            id          = other.id;
            _handle     = other._handle;
            _bindings   = other._bindings;
            _attributes = other._attributes;
            _ebo        = other._ebo;

            other.id      = 0;
            other._handle = 0;
        }
        ~GLVao();

        void bind() const;
        bool is_bound() const { return _is_bound; }
        bool uses_ebo() const { return _ebo.id != 0u; }

        void update_binding(uint32_t binding_id, uint32_t new_handle);
        void update_ebo(uint32_t new_handle);

      private:
        void _calculate_attr_offsets_if_zeros();
        void _configure_bindings();
        void _configure_attributes();
        uint32_t _get_gl_attr_format(ATTR_FORMAT format);

        uint32_t _handle;
        std::vector<GLVaoBinding> _bindings;
        std::vector<GLVaoAttribute> _attributes;
        Handle<GLBuffer> _ebo;

        bool _is_bound{false};
    };
} // namespace eng
