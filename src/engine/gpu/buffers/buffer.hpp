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
    struct GLBufferDescriptor {
        GLBufferDescriptor() = default;
        explicit GLBufferDescriptor(uint32_t flags) : flags{flags} {}
        bool operator==(const GLBufferDescriptor &other) const noexcept {
            return handle == other.handle;
        }

        std::uint32_t handle{0u}, flags{0u};
        std::size_t size{0u}, capacity{0u};
    };

    struct GLBuffer {
        GLBuffer() = default;
        explicit GLBuffer(GLBuffer &&) noexcept;
        GLBuffer &operator=(GLBuffer &&) noexcept;
        ~GLBuffer();

        explicit GLBuffer(uint32_t flags);
        GLBuffer(void *data, uint32_t size_bytes, uint32_t flags);
        template <typename T>
        GLBuffer(std::vector<T> &data, uint32_t flags)
            : GLBuffer(data.data(), data.size() * sizeof(T), flags) {}

        void push_data(const void *data, size_t size_bytes);
        void clear_invalidate();

        GLBufferDescriptor descriptor;
        Signal<const GLBufferDescriptor &> on_handle_change;

      private:
        void resize(size_t required_size);

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
        ~GLVao();

        void bind() const;
        bool is_bound() const { return _is_bound; }
        bool uses_ebo() const { return _uses_ebo; }

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
        bool _uses_ebo{false};
    };
} // namespace eng
