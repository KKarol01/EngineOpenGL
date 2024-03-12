#include "postprocess.hpp"

#include <engine/engine.hpp>

namespace eng {
    PostprocessBloom::PostprocessBloom(uint32_t number_of_passes) : _pass_num{number_of_passes} {
        for (auto i = 0u; i < _pass_num; ++i) {
            _pass_textures.push_back(Engine::instance().get_gpu_res_mgr()->create_resource(
                Texture{TextureSettings{GL_R11F_G11F_B10F, GL_CLAMP_TO_EDGE, GL_LINEAR, 1},
                        TextureImageDataDescriptor{"", 1920 / ((int)i + 2), 1080 / ((int)i + 2)}}));
        }
        _pass_fbo = Framebuffer{
            {FramebufferAttachment{GL_COLOR_ATTACHMENT0, _pass_textures[0]->res_handle()}}};
        down_sample = Engine::instance().get_gpu_res_mgr()->create_resource(ShaderProgram{"bloom"});
        up_sample
            = Engine::instance().get_gpu_res_mgr()->create_resource(ShaderProgram{"bloom_up"});
    }

    void PostprocessBloom::render(Texture *hdr_color, GLVao *quad_vao) {
        quad_vao->bind();
        _pass_fbo.bind();
        down_sample->use();
        hdr_color->bind(0);
        for (auto i = 0u; i < _pass_num; ++i) {
            glMemoryBarrier(GL_FRAMEBUFFER_BARRIER_BIT);
            if (i > 0u) { _pass_textures[i - 1]->bind(0); }
            auto txt = _pass_textures[i];
            _pass_fbo.update_attachments(
                {FramebufferAttachment{GL_COLOR_ATTACHMENT0, txt->res_handle()}});
            glViewport(0, 0, txt->get_size().first, txt->get_size().second);
            glClear(GL_COLOR_BUFFER_BIT);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }

        up_sample->use();
        up_sample->set("filterRadius", 0.0005f);
        hdr_color->bind(1);

        for (int i = _pass_num - 2; i >= -1; --i) {
            glMemoryBarrier(GL_FRAMEBUFFER_BARRIER_BIT);
            if (i > -1) {
                _pass_textures[i + 1]->bind(0);
                auto txt = _pass_textures[i];
                _pass_fbo.update_attachments(
                    {FramebufferAttachment{GL_COLOR_ATTACHMENT0, txt->res_handle()}});
                up_sample->set("primary", 0.0f);
                glViewport(0, 0, txt->get_size().first, txt->get_size().second);
            } else {
                _pass_fbo.update_attachments(
                    {FramebufferAttachment{GL_COLOR_ATTACHMENT0, hdr_color->res_handle()}});
                up_sample->set("primary", 1.0f);
                glViewport(0, 0, 1920, 1080);
            }

            if (i > -1) { glClear(GL_COLOR_BUFFER_BIT); }
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }
    }
} // namespace eng
