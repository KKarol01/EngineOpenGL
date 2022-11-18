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

        struct UBO_CLIENTDATA {
            glm::mat4 p, v, m;
            glm::vec4 cam_dir, cam_pos, light_dir, light_col;
            float attenuation;
            int use_pbr;
        } client_data;
        client_data.attenuation = 12.f;
        client_data.use_pbr     = 1;
        client_data.light_dir   = glm::vec4{1.f};
        client_data.light_col   = glm::vec4{1.f};
        GLBuffer ubo{&client_data,
                     264,
                     GL_MAP_WRITE_BIT | GL_MAP_READ_BIT | GL_MAP_COHERENT_BIT | GL_MAP_PERSISTENT_BIT
                         | GL_DYNAMIC_STORAGE_BIT};
        auto ubo_map = (char *)glMapNamedBufferRange(ubo.descriptor.handle,
                                                     0,
                                                     sizeof(UBO_CLIENTDATA),
                                                     GL_MAP_WRITE_BIT | GL_MAP_READ_BIT | GL_MAP_COHERENT_BIT
                                                         | GL_MAP_PERSISTENT_BIT);

        UBO u{{{"time", (float)glfwGetTime()}}};

        ModelImporter imp;
        auto sphere = imp.import_model("3dmodels/sphere/sphere.glb",
                                       aiProcess_Triangulate | aiProcess_DropNormals | aiProcess_GenSmoothNormals
                                           | aiProcess_JoinIdenticalVertices);

        glm::vec3 cc{0.3f};
        Camera::LensSettings lens = cam.lens;
        engine.gui_->add_draw([&] {
            ImGui::SliderFloat3("light position", (float *)(ubo_map + 224), -3.f, 3.f);
            ImGui::ColorEdit3("light color", (float *)(ubo_map + 240));
            ImGui::ColorEdit3("Background color", &cc.x);
            ImGui::SliderFloat("light attenuation", (float *)(ubo_map + 256), 0.1f, 1000.f);
            ImGui::Checkbox("Use physically-based rendering", (bool *)(ubo_map + 260));
            if (ImGui::Button("recompile PBR shader")) { pbr_program.recompile(); }
            if (ImGui::Button("recompile def shader")) { default_program.recompile(); }
            if (ImGui::Button("recompile refr shader")) { rect_program.recompile(); }

            if (ImGui::CollapsingHeader("Camera lens settings")) {
                ImGui::SliderFloat("FoV", &lens.fovydeg, 0.f, 90.f);
                cam.adjust_lens(lens);
            }
        });

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

            auto p  = cam.perspective_matrix();
            auto v  = cam.view_matrix();
            auto m  = mat_model;
            auto cd = cam.forward_vec();
            auto cp = cam.position();
            memcpy((char *)ubo_map + 0, &p, 64);
            memcpy((char *)ubo_map + 64, &v, 64);
            memcpy((char *)ubo_map + 128, &m, 64);
            memcpy((char *)ubo_map + 192, &cd, 16);
            memcpy((char *)ubo_map + 208, &cp, 16);

            rect_program.use();
            *u.get<float>("time") = (float)glfwGetTime();
            u.bind(0);
            re.get_vao(rectvao).bind();
            glBindTextureUnit(0, t.handle);
            glDrawArrays(GL_TRIANGLES, 0, 6);

            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glBindBufferRange(GL_UNIFORM_BUFFER, 2, ubo.descriptor.handle, 0, sizeof(UBO_CLIENTDATA));
            default_program.use();
            default_program.set("time", time);
            re.get_vao(vaoid).bind();
            glDrawElements(GL_TRIANGLES, sphere.indices.size(), GL_UNSIGNED_INT, 0);
            glDisable(GL_BLEND);
            // eng::Engine::instance().gui_->draw();
            window->swap_buffers();
        }
    } catch (const std::runtime_error &e) { printf("%s\n", e.what()); }

    return 0;
}
