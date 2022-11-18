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

#include "engine/subsystems/gui/gui.hpp"
#include "engine/model/importers.hpp"
#include "engine/renderer/renderer.hpp"
#include "engine/wrappers/buffer/ubo.hpp"

int main() {
    eng::Engine::initialise({Window{"window", 1920, 1080}});
    auto &engine      = eng::Engine::instance();
    const auto window = engine.window();
    auto &re          = *engine.renderer_.get();
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    try {
        Shader default_program{"default"};
        Shader pbr_program{"indirect"};
        Shader rect_program{"refr"};
        Camera cam;
        glm::mat4 mat_model = glm::scale(glm::mat4{1.f}, glm::vec3{1.f});

        UBO u{{{"time", (float)glfwGetTime()}}};
        UBO pbrubo{{{"p", glm::mat4{1.}},
                    {"v", glm::mat4{1.}},
                    {"m", glm::mat4{1.}},
                    {"cd", glm::vec4{1.f}},
                    {"cp", glm::vec4{1.f}},
                    {"lp", glm::vec4{1.f}},
                    {"lc", glm::vec4{1.f}},
                    {"att", 1.f},
                    {"usepbr", 1}}};

        UBO ubo_fire_settings1{{{"clip", 80.f},
                                {"xatt", .7f},
                                {"speed", 1.5f},
                                {"scale", 10.f},
                                {"flowxmult", 3.f},
                                {"flowymult", -2.f},
                                {"flow_dir", glm::vec4{.0f, -2.f, 1.f, .0f}},
                                {"dsp", glm::vec4{3.f, .5f, 2.f, .0f}},
                                {"dspmult", 2.f},
                                {"dsp3_noiseatt", .3f},
                                {"noiseres", glm::vec4{3.f, 1.f, 2.f, .0f}},
                                {"noisedsp3factor", .4f},
                                {"noisesmoothness", 1.f},
                                {"color_weights_mult", glm::vec4{1.f, 2.f, 3.f, 1.5f}},
                                {"alpha_threshold", .45f}}};

        ModelImporter imp;
        auto sphere = imp.import_model("3dmodels/sphere/sphere.glb",
                                       aiProcess_Triangulate | aiProcess_DropNormals | aiProcess_GenSmoothNormals
                                           | aiProcess_JoinIdenticalVertices);

        glm::vec3 cc{0.3f};
        Camera::LensSettings lens = cam.lens;
        engine.gui_->add_draw([&] {
            ImGui::SliderFloat3("light position", pbrubo.get<float>("lp"), -3.f, 3.f);
            ImGui::ColorEdit3("light color", pbrubo.get<float>("lc"));
            ImGui::ColorEdit3("Background color", &cc.x);
            ImGui::SliderFloat("light attenuation", pbrubo.get<float>("att"), 0.1f, 1000.f);
            ImGui::Checkbox("Use physically-based rendering", pbrubo.get<bool>("usepbr"));
            if (ImGui::Button("recompile PBR shader")) { pbr_program.recompile(); }
            if (ImGui::Button("recompile def shader")) { default_program.recompile(); }
            if (ImGui::Button("recompile refr shader")) { rect_program.recompile(); }

            if (ImGui::CollapsingHeader("Camera lens settings")) {
                ImGui::SliderFloat("FoV", &lens.fovydeg, 0.f, 90.f);
                cam.adjust_lens(lens);
            }

            if (ImGui::CollapsingHeader("fire settings", ImGuiTreeNodeFlags_DefaultOpen)) {
                ImGui::SliderFloat("clip", ubo_fire_settings1.get<float>("clip"), 0.f, 200.f);
                ImGui::SliderFloat("xatt", ubo_fire_settings1.get<float>("xatt"), 0.f, 1.f);
                ImGui::SliderFloat("speed", ubo_fire_settings1.get<float>("speed"), 0.f, 10.f);
                ImGui::SliderFloat("scale", ubo_fire_settings1.get<float>("scale"), 1.f, 50.f);
                ImGui::SliderFloat("flowxmult", ubo_fire_settings1.get<float>("flowxmult"), 0.f, 5.f);
                ImGui::SliderFloat("flowymult", ubo_fire_settings1.get<float>("flowymult"), -5.f, 5.f);
                ImGui::SliderFloat3("flow_dir", ubo_fire_settings1.get<float>("flow_dir"), -3.f, 3.f);
                ImGui::SliderFloat3("dsp", ubo_fire_settings1.get<float>("dsp"), -5.f, 5.f);
                ImGui::SliderFloat("dspmult", ubo_fire_settings1.get<float>("dspmult"), 0.f, 5.f);
                ImGui::SliderFloat("dsp3_noiseatt", ubo_fire_settings1.get<float>("dsp3_noiseatt"), 0.f, 1.f);
                ImGui::SliderFloat3("noiseres", ubo_fire_settings1.get<float>("noiseres"), -5.f, 5.f);
                ImGui::SliderFloat("noisedsp3factor", ubo_fire_settings1.get<float>("noisedsp3factor"), 0.f, 2.f);
                ImGui::SliderFloat("noisesmoothness", ubo_fire_settings1.get<float>("noisesmoothness"), -5.f, 5.f);
                ImGui::SliderFloat4(
                    "color_weights_mult", ubo_fire_settings1.get<float>("color_weights_mult"), 0., 10.f);
                ImGui::SliderFloat("alpha_threshold", ubo_fire_settings1.get<float>("alpha_threshold"), 0.f, 3.f);
            }
        });
        std::cout << *ubo_fire_settings1.get<float>("alpha_threshold");
        auto vbufferid = re.create_buffer(GLBufferDescriptor{GL_DYNAMIC_STORAGE_BIT});
        auto ebufferid = re.create_buffer(GLBufferDescriptor{GL_DYNAMIC_STORAGE_BIT});
        auto vaoid     = re.create_vao(GLVaoDescriptor{{GLVaoBinding{0, vbufferid, 32}},
                                                   GLVaoAttrCustom{{
                                                       ATTRCUSTORMFORMAT{0, 0, 3, GL_FLOAT, 4, 0},
                                                       ATTRCUSTORMFORMAT{1, 0, 3, GL_FLOAT, 4, 12},
                                                       ATTRCUSTORMFORMAT{2, 0, 2, GL_FLOAT, 4, 24},
                                                   }},
                                                   ebufferid});
        {
            auto &vbuffer = re.get_buffer(vbufferid);
            auto &ebuffer = re.get_buffer(ebufferid);

            vbuffer.push_data(sphere.vertices.data(), sphere.vertices.size() * 4);
            ebuffer.push_data(sphere.indices.data(), sphere.indices.size() * 4);
        }

        Texture t{FILTER_SETTINGS{GL_LINEAR}, WRAP_SETTINGS{GL_CLAMP_TO_EDGE}};
        t.build2d("textures/deco.jpg", GL_RGBA8);
        std::cout << glGetError() << '\n';
        auto rectvbo = re.create_buffer(GLBufferDescriptor{GL_DYNAMIC_STORAGE_BIT});
        auto rectvao = re.create_vao(GLVaoDescriptor{{GLVaoBinding{0, rectvbo, 8}},
                                                     GLVaoAttrCustom{{ATTRCUSTORMFORMAT{0, 0, 2, GL_FLOAT, 4, 0}}}});

        {
            float rectvert[]{0.f, 0.f, 1.f, 0.f, 0.f, 1.f, 1.f, 0.f, 1.f, 1.f, 0.f, 1.f};
            re.get_buffer(rectvbo).push_data(rectvert, sizeof(rectvert));
        }

        while (!window->should_close()) {
            float time = glfwGetTime();
            glfwPollEvents();
            glClearColor(cc.x, cc.y, cc.z, 1.);
            window->clear_framebuffer();
            eng::Engine::instance().controller()->update();
            cam.update();

            pbrubo.set("p", cam.perspective_matrix());
            pbrubo.set("v", cam.view_matrix());
            pbrubo.set("m", mat_model);
            pbrubo.set("cd", cam.forward_vec());
            pbrubo.set("cp", cam.position());

            /*rect_program.use();
            u.set("time", (float)glfwGetTime());
            u.bind(0);
            re.get_vao(rectvao).bind();
            glBindTextureUnit(0, t.handle);
            glDrawArrays(GL_TRIANGLES, 0, 6);*/

            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            pbrubo.bind(2);
            ubo_fire_settings1.bind(5);
            default_program.use();
            default_program.set("time", time);
            re.get_vao(vaoid).bind();
            glDrawElements(GL_TRIANGLES, sphere.indices.size(), GL_UNSIGNED_INT, 0);
            glDisable(GL_BLEND);
            eng::Engine::instance().gui_->draw();
            window->swap_buffers();
        }
    } catch (const std::runtime_error &e) { printf("%s\n", e.what()); }

    return 0;
}
