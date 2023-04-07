#pragma once

#include <vector>
#include <unordered_map>

#include "../../renderer/typedefs.hpp"

namespace eng {
    typedef uint32_t GLFboAttachment;

    struct GLFboDescriptor {
        GLFboDescriptor();
        GLFboDescriptor(GLFboDescriptor &&other) noexcept;
        GLFboDescriptor &operator=(GLFboDescriptor &&other) noexcept;
        ~GLFboDescriptor();

        FboID handle{0u};
        std::unordered_map<GLFboAttachment, TextureID> attachments;
    };

    struct GLFbo {
        
        void add_attachment();

        GLFboDescriptor descriptor;
    };
} // namespace eng