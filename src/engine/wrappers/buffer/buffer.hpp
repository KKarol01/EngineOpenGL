#pragma once

#include <stdint.h>
#include <vector>
#include <initializer_list>
#include <cstdint>
#include <functional>
#include <map>
#include <variant>
#include <cassert>

#include "../../signal/signal.hpp"
#include "../../types/types.hpp"

namespace eng {
    struct GLBufferDescriptor {
        GLBufferDescriptor() = default;
        explicit GLBufferDescriptor(uint32_t flags) : flags{flags} {}
        bool operator==(const GLBufferDescriptor &other) const noexcept { return handle == other.handle; }

        std::uint32_t handle{0u}, flags{0u};
        std::size_t size{0u}, capacity{0u};

        Signal<uint32_t> on_handle_change;
    };

    struct GLBuffer {
        GLBuffer() = default;
        GLBuffer(GLBuffer &&) noexcept;
        GLBuffer &operator=(GLBuffer &&) noexcept;
        ~GLBuffer();

        explicit GLBuffer(GLBufferDescriptor desc);
        GLBuffer(void *data, uint32_t size_bytes, GLBufferDescriptor desc);
        template <typename T>
        GLBuffer(std::vector<T> &data, GLBufferDescriptor desc)
            : GLBuffer(data.data(), data.size() * sizeof(T), desc) {}

        void push_data(const void *data, size_t size_bytes);

        GLBufferDescriptor descriptor;

      private:
        void resize(size_t required_size);

        static constexpr float GROWTH_FACTOR{1.61f};
    };

    enum GL_FORMAT_ { GL_FORMAT_FLOAT };

    struct GLVaoAttributeDescriptor {
        GLVaoAttributeDescriptor(uint32_t idx, uint32_t binding, uint32_t size, uint32_t offset);
        GLVaoAttributeDescriptor(
            uint32_t idx, uint32_t binding, uint32_t size, uint32_t offset, GL_FORMAT_ gl_format, bool normalize);

        uint32_t idx, size{0}, offset{0};
        uint32_t binding;
        bool normalize{false};
        GL_FORMAT_ gl_format = GL_FORMAT_FLOAT;
    };

    struct GLVaoAttributeSameType {
        GLVaoAttributeSameType(uint32_t idx, uint32_t binding, uint32_t size, bool normalize = false)
            : idx{idx}, binding{binding}, size{size}, normalize{normalize} {}

        uint32_t idx, binding, size;
        bool normalize{false};
    };
    struct GLVaoAttributeSameFormat {
        GLVaoAttributeSameFormat(uint32_t idx, uint32_t binding, bool normalize = false)
            : idx{idx}, binding{binding}, normalize{normalize} {}

        uint32_t idx, binding;
        bool normalize{false};
    };

    struct GLVaoBufferBinding {
        uint32_t binding;
        BufferID buffer;
    };

    struct GLVaoDescriptor {
        GLVaoDescriptor() = default;

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

        std::vector<GLVaoAttributeDescriptor> attributes;
        std::vector<GLVaoBufferBinding> buff_bindings;
        BufferID ebo_buffer{0};
        bool ebo_set = false;
    };

    struct GLVao {
        GLVao();
        GLVao(GLVao &&) noexcept;
        GLVao &operator=(GLVao &&) noexcept;
        bool operator==(const GLVao &other) const noexcept { return handle == other.handle; }
        ~GLVao();

        void bind() const;

        void configure_binding(uint32_t id, BufferID bufferid, size_t stride, size_t offset = 0u);
        void configure_ebo(BufferID bufferid);

        void configure_attributes(const std::vector<GLVaoAttributeDescriptor> &attributes);
        void configure_attributes(uint32_t size,
                                  GL_FORMAT_ gl_format,
                                  uint32_t type_size_bytes,
                                  const std::vector<GLVaoAttributeSameFormat> &attributes);
        void configure_attributes(GL_FORMAT_ gl_format,
                                  uint32_t type_size_bytes,
                                  const std::vector<GLVaoAttributeSameType> &attributes);

        GLVaoDescriptor descriptor;

      private:
        uint32_t handle;
        void configure_attributes();
    };
} // namespace eng
