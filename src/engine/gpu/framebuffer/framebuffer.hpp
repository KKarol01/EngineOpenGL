#pragma once

#include <vector>
#include <unordered_map>
#include <utility>

#include <engine/types/idresource.hpp>
#include <engine/gpu/texture/texture.hpp>

namespace eng {

    struct FramebufferAttachment {
        uint32_t target{0u};
        Handle<Texture> texture{0u};
        uint32_t level{0u};
    };

    class Framebuffer : public IdResource<Framebuffer> {
      public:
        explicit Framebuffer() = default;
        explicit Framebuffer(std::initializer_list<FramebufferAttachment> texture_attachments);
        Framebuffer(Framebuffer &&) noexcept;
        Framebuffer &operator=(Framebuffer &&) noexcept;
        ~Framebuffer() override;

        void bind();
        uint32_t handle() { return _handle; }

        void update_attachments(std::initializer_list<FramebufferAttachment> texture_attachments);

      private:
        void _configure_attachments();
        void _assert_completness();

        uint32_t _handle{0u};
        std::unordered_map<uint32_t, FramebufferAttachment> _attachments;
    };
} // namespace eng