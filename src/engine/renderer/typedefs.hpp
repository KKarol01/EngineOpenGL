#pragma once

#include <cstdint>

namespace eng {
    typedef uint32_t GLBufferID;
    typedef uint32_t GLVaoID;
    typedef uint32_t TextureID;
    typedef uint32_t FboID;
    typedef uint32_t ProgramID;
    typedef uint32_t PipelineID;

    typedef uint32_t RenderResourceID;

    struct RendererResource {
        RenderResourceID id{0u};
    };

} // namespace eng