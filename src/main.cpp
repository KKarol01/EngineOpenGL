#include <filesystem>
#include <ranges>
#include <array>
#include <memory>
#include <numeric>

#include "engine/engine.hpp"
#include "engine/wrappers/include_all.hpp"
#include "engine/wrappers/texture/texture.hpp"
#include "engine/camera/camera.hpp"
#include "engine/subsystems/ecs/components.hpp"
#include "engine/subsystems/ecs/ecs.hpp"

#include "engine/controller/keyboard/keyboard.hpp"
#include "engine/types/types.hpp"

#include <GLFW/glfw3.h>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <stb_image.h>
#include <imgui/imgui.h>
#include <glm/gtx/euler_angles.hpp>

#include "engine/subsystems/gui/gui.hpp"
#include "engine/model/importers.hpp"
#include "engine/renderer/renderer.hpp"
#include "engine/wrappers/buffer/ubo.hpp"

int main() {
    eng::Engine::initialise({Window{"window", 1920, 1080}});
    auto &engine      = eng::Engine::instance();
    const auto window = engine.window();
    auto &re          = *engine.renderer_.get();
    Camera cam;
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    glm::vec3 cc{0.4f};
    Shader vol{"vol"};
    Shader volfill{"volfill"};
    glm::vec3 transform{0.f}, scale{1.f};
    glm::mat4 model{1.f};

    engine.gui_->add_draw([&] {
        if (ImGui::Button("recompile shader")) { vol.recompile(); }
        if (ImGui::Button("recompile comp shader")) { volfill.recompile(); }

        ImGui::SliderFloat3("t", &transform.x, -5.f, 5.f);
        ImGui::SliderFloat3("s", &scale.x, 0.01f, 5.f);
        model = glm::translate(glm::scale(glm::mat4{1.f}, scale), transform);

        ImGui::ColorEdit3("background color", &cc.x);
    });

    auto rectvbo = re.create_buffer({0});
    auto rectebo = re.create_buffer({0});
    {
        float verts[]{0.f, 0.f, 1.f, 0.f, 0.f, 1.f, 1.f, 1.f};
        unsigned inds[]{0, 1, 2, 3};

        auto &rvbo = re.get_buffer(rectvbo);
        auto &rebo = re.get_buffer(rectebo);

        rvbo.push_data(verts, sizeof(verts));
        rebo.push_data(inds, sizeof(inds));
    }
    auto rectvao = re.create_vao(GLVaoDescriptor{
        {GLVaoBinding{0, rectvbo, 8, 0}}, GLVaoAttrSameFormat{2, GL_FLOAT, 4, {ATTRSAMEFORMAT{0, 0}}}, rectebo});

    uint32_t tex;
    glCreateTextures(GL_TEXTURE_3D, 1, &tex);
    glTextureStorage3D(tex, 1, GL_R32F, 128, 128, 128);
    glTextureParameteri(tex, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(tex, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTextureParameteri(tex, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTextureParameteri(tex, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(tex, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    while (!window->should_close()) {
        float time = glfwGetTime();
        glfwPollEvents();
        glClearColor(cc.x, cc.y, cc.z, 1.);
        window->clear_framebuffer();
        eng::Engine::instance().controller()->update();
        cam.update();


        //volfill.use();
        //volfill.set("time", time);
        //glBindImageTexture(0, tex, 0, GL_TRUE, 0, GL_READ_WRITE, GL_R32F);
        //glDispatchCompute(8, 8, 32);
        //glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        //glBindImageTexture(0, 0, 0, GL_TRUE, 0, GL_READ_WRITE, GL_R32F);
        //

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        vol.use();
        vol.set("projection", cam.perspective_matrix());
        vol.set("time", time);
        vol.set("view", cam.view_matrix());
        vol.set("cam_view", cam.forward_vec());
        vol.set("cam_pos", cam.position());
        vol.set("model", model);
        re.get_vao(rectvao).bind();
        glBindTextureUnit(0, tex);
        glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_INT, 0);
        glDisable(GL_BLEND);

        eng::Engine::instance().gui_->draw();
        window->swap_buffers();
    }

    return 0;
}
