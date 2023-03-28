#pragma once

#include <cstdint>
#include <vector>
#include <initializer_list>
#include <cstdint>
#include <functional>
#include <map>
#include <variant>
#include <cassert>

#include "../../types/types.hpp"
#include "../../renderer/typedefs.hpp"

namespace eng {
    struct GLBufferDescriptor {
        GLBufferDescriptor() = default;
        explicit GLBufferDescriptor(uint32_t flags) : flags{flags} {}
        bool operator==(const GLBufferDescriptor &other) const noexcept { return handle == other.handle; }

        BufferID id{gid++};
        std::uint32_t handle{0u}, flags{0u};
        std::size_t size{0u}, capacity{0u};

      private:
        static inline BufferID gid{1u};
    };

    struct GLBuffer {
        GLBuffer() = default;
        GLBuffer(GLBuffer &&) noexcept;
        GLBuffer &operator=(GLBuffer &&) noexcept;
        ~GLBuffer();

        explicit GLBuffer(uint32_t flags);
        GLBuffer(void *data, uint32_t size_bytes, uint32_t flags);
        template <typename T>
        GLBuffer(std::vector<T> &data, uint32_t flags) : GLBuffer(data.data(), data.size() * sizeof(T), flags) {}

        void push_data(const void *data, size_t size_bytes);

        GLBufferDescriptor descriptor;
        Signal<const GLBufferDescriptor &> on_handle_change;

      private:
        void resize(size_t required_size);

        static constexpr float GROWTH_FACTOR{1.61f};
    };

    enum GL_FORMAT_ { GL_FORMAT_FLOAT };
    enum GL_ATTR_ {
        GL_ATTR_0,
        GL_ATTR_1,
        GL_ATTR_2,
        GL_ATTR_3,
        GL_ATTR_4,
        GL_ATTR_5,
        GL_ATTR_6,
    };

    struct GLVaoAttributeDescriptor {
        GLVaoAttributeDescriptor(GL_ATTR_ idx, uint32_t binding, uint32_t size, uint32_t offset);
        GLVaoAttributeDescriptor(
            GL_ATTR_ idx, uint32_t binding, uint32_t size, uint32_t offset, GL_FORMAT_ gl_format, bool normalize);

        GL_ATTR_ idx;
        uint32_t size{0}, offset{0};
        uint32_t binding;
        bool normalize{false};
        GL_FORMAT_ gl_format = GL_FORMAT_FLOAT;
    };

    // struct GLVaoAttributeSameType {
    //     GLVaoAttributeSameType(uint32_t idx, uint32_t binding, uint32_t size, bool normalize = false)
    //         : idx{idx}, binding{binding}, size{size}, normalize{normalize} {}

    //    uint32_t idx, binding, size;
    //    bool normalize{false};
    //};
    // struct GLVaoAttributeSameFormat {
    //    GLVaoAttributeSameFormat(uint32_t idx, uint32_t binding, bool normalize = false)
    //        : idx{idx}, binding{binding}, normalize{normalize} {}

    //    uint32_t idx, binding;
    //    bool normalize{false};
    //};

    struct GLVaoBufferBinding {
        uint32_t binding{0u};
        BufferID buffer{0u};
        uint32_t stride{0u}, offset{0u};
    };

    struct GLVaoDescriptor {
        GLVaoDescriptor() = default;

        bool operator==(const GLVaoDescriptor &other) const noexcept { return handle == other.handle; }

        const GLVaoAttributeDescriptor &get_attrib_by_binding(uint32_t binding) const {
            for (auto &attr : attributes) {
                if (attr.binding == binding) { return attr; }
            }

            assert(("Non-existent binding" && false));
            return *attributes.begin();
        }
        GLVaoAttributeDescriptor &get_attrib_by_binding(uint32_t binding) {
            for (auto &attr : attributes) {
                if (attr.binding == binding) { return attr; }
            }

            assert(("Non-existent binding" && false));
            return *attributes.begin();
        }

        uint32_t handle{0u};
        std::vector<GLVaoAttributeDescriptor> attributes;
        std::vector<GLVaoBufferBinding> buff_bindings;
        BufferID ebo_buffer_id{0u};
    };

    struct GLVao {
        GLVao();
        GLVao(GLVao &&) noexcept;
        GLVao &operator=(GLVao &&) noexcept;
        ~GLVao();

        void bind() const;

        void configure_binding(uint32_t id, BufferID bufferid, size_t stride, size_t offset = 0u);
        void configure_ebo(BufferID bufferid);

        void configure_attribute(GL_ATTR_ attribute_idx,
                                  uint32_t binding_id,
                                  uint32_t size,
                                  uint32_t byte_offset,
                                  GL_FORMAT_ format = GL_FORMAT_FLOAT,
                                  bool normalize    = false);

        GLVaoDescriptor descriptor;
    };
} // namespace eng
