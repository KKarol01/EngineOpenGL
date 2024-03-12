#pragma once

#include <engine/gpu/texture/texture.hpp>
#include <engine/gpu/framebuffer/framebuffer.hpp>
#include <engine/gpu/shaderprogram/shader.hpp>
#include <engine/gpu/buffers/buffer.hpp>

namespace eng {
    class Postprocess {
      public:
        virtual ~Postprocess() = default;
    };

    class PostprocessBloom : public Postprocess {
      public:
        explicit PostprocessBloom() = default;
        explicit PostprocessBloom(uint32_t number_of_passes = 4);

        void render(Texture *hdr_color, GLVao *quad_vao);

      private:
        uint32_t _pass_num{4};
        ShaderProgram *down_sample, *up_sample;
        Framebuffer _pass_fbo;
        std::vector<Texture *> _pass_textures;
    };
} // namespace eng