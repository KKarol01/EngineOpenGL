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

    glm::vec3 cc{0.4f};
    Shader pbr{"indirect"};
    Shader vol{"vol"};
    Shader volfill{"volfill"};
    glm::vec3 transform{0.f}, scale{1.f};
    glm::mat4 model{1.f};

    glm::mat4 katanamat{1.f}, holdermat{1.f};

    Model katana  = ModelImporter{}.import_model("3dmodels/torch/scene.gltf",
                                                aiProcess_Triangulate | aiProcess_CalcTangentSpace | aiProcess_FlipUVs);
    Model katana2 = ModelImporter{}.import_model(
        "3dmodels/katana/scene.gltf", aiProcess_Triangulate | aiProcess_CalcTangentSpace | aiProcess_FlipUVs);

    typedef struct {
        uint32_t count;
        uint32_t instanceCount;
        uint32_t firstIndex;
        int baseVertex;
        uint32_t baseInstance;
    } DrawElementsIndirectCommand;

    auto kc = re.create_buffer({GL_DYNAMIC_STORAGE_BIT});
    auto kt = re.create_buffer({GL_DYNAMIC_STORAGE_BIT});
    auto km = re.create_buffer(
        {GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT | GL_MAP_READ_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT});
    VaoID kv;
    {
        auto kb = re.create_buffer({GL_DYNAMIC_STORAGE_BIT});
        auto ke = re.create_buffer({GL_DYNAMIC_STORAGE_BIT});

        re.get_buffer(kb).push_data(katana.vertices.data(), katana.vertices.size() * 4);
        re.get_buffer(ke).push_data(katana.indices.data(), katana.indices.size() * 4);
        re.get_buffer(kb).push_data(katana2.vertices.data(), katana2.vertices.size() * 4);
        re.get_buffer(ke).push_data(katana2.indices.data(), katana2.indices.size() * 4);

        kv = re.create_vao(GLVaoDescriptor{{GLVaoBinding{0, kb, 14 * 4, 0}},
                                           GLVaoAttrSameType{GL_FLOAT,
                                                             4,
                                                             {ATTRSAMETYPE{0, 0, 3},
                                                              ATTRSAMETYPE{1, 0, 3},
                                                              ATTRSAMETYPE{2, 0, 3},
                                                              ATTRSAMETYPE{3, 0, 3},
                                                              ATTRSAMETYPE{4, 0, 2}}},
                                           ke});
        std::vector<size_t> bindless_handles;

        for (auto &t : katana.meshes) {
            bindless_handles.push_back(katana.textures[t.textures[t.DIFFUSE]].bindless_handle);
            bindless_handles.push_back(katana.textures[t.textures[t.NORMAL]].bindless_handle);
            bindless_handles.push_back(katana.textures[t.textures[t.METALNESS]].bindless_handle);
            bindless_handles.push_back(katana.textures[t.textures[t.ROUGHNESS]].bindless_handle);
        }
        for (auto &t : katana2.meshes) {
            bindless_handles.push_back(katana2.textures[t.textures[t.DIFFUSE]].bindless_handle);
            bindless_handles.push_back(katana2.textures[t.textures[t.NORMAL]].bindless_handle);
            bindless_handles.push_back(katana2.textures[t.textures[t.METALNESS]].bindless_handle);
            bindless_handles.push_back(katana2.textures[t.textures[t.ROUGHNESS]].bindless_handle);
        }

        re.get_buffer(kt).push_data(bindless_handles.data(), bindless_handles.size() * sizeof(size_t));

        std::vector<DrawElementsIndirectCommand> katana_commands;

        auto boff{0u}, ioff{0u};
        for (auto i = 0; i < katana.meshes.size(); ++i) {
            const auto &k = katana.meshes[i];
            katana_commands.emplace_back(k.index_count, 1, k.index_offset, k.vertex_offset, 0);
            // zle offsety sa liczone
            boff += k.vertex_count;
            ioff += k.index_count;
        }
        for (auto i = 0; i < katana2.meshes.size(); ++i) {
            const auto &k = katana2.meshes[i];
            katana_commands.emplace_back(k.index_count, 1, ioff + k.index_offset, boff + k.vertex_offset, 0);
            // zle offsety sa liczone
        }

        re.get_buffer(kc).push_data(katana_commands.data(),
                                    sizeof(DrawElementsIndirectCommand) * katana_commands.size());
    }
    UBO pbr_ubo{{
        {"p", glm::mat4{1.f}},
        {"v", glm::mat4{1.f}},
        {"m", glm::mat4{1.f}},
        {"cam_dir", glm::vec4{1.f}},
        {"cam_pos", glm::vec4{1.f}},
        {"lpos", glm::vec4{1.f}},
        {"lcol", glm::vec4{1.f}},
        {"attenuation", 1.f},
        {"use_pbr", 1},
    }};

    re.get_buffer(km).push_data(0, 64 * 2);
    auto kmptr = glMapNamedBufferRange(
        re.get_buffer(km).descriptor.handle, 0, 128, GL_MAP_WRITE_BIT | GL_MAP_COHERENT_BIT | GL_MAP_PERSISTENT_BIT);

    engine.gui_->add_draw([&] {
        if (ImGui::Button("recompile shader")) { vol.recompile(); }
        if (ImGui::Button("recompile comp shader")) { volfill.recompile(); }

        ImGui::SliderFloat3("t", &transform.x, -5.f, 5.f);
        ImGui::SliderFloat3("s", &scale.x, 0.01f, 5.f);
        model = glm::translate(glm::scale(glm::mat4{1.f}, scale), transform);

        ImGui::SliderFloat("attenuation", pbr_ubo.get<float>("attenuation"), .0f, 32.f);
        ImGui::SliderFloat3("light_pos", pbr_ubo.get<float>("lpos"), -.5, 5.f);
        ImGui::ColorPicker3("light_color", pbr_ubo.get<float>("lcol"));
        ImGui::SliderFloat3("holder pos", &holdermat[3].x, -.5, 5.f);
        ImGui::SliderFloat("holder scale", &holdermat[0].x, -5., 5.f);
        ImGui::SliderFloat3("katana pos", &katanamat[3].x, -5., 5.f);
        ImGui::SliderFloat("katana scale", &katanamat[0].x, -.5, 5.f);
        katanamat[1].y = katanamat[0].x;
        katanamat[2].z = katanamat[0].x;
        holdermat[1].y = holdermat[0].x;
        holdermat[2].z = holdermat[0].x;

        memcpy(kmptr, &holdermat, 64);
        memcpy((char *)kmptr + 64, &katanamat, 64);

        ImGui::ColorEdit3("background color", &cc.x);
    });
    auto rectvbo = re.create_buffer({0});
    auto rectebo = re.create_buffer({0});
    auto rectvao = re.create_vao(GLVaoDescriptor{
        {GLVaoBinding{0, rectvbo, 8, 0}}, GLVaoAttrSameFormat{2, GL_FLOAT, 4, {ATTRSAMEFORMAT{0, 0}}}, rectebo});
    {
        float verts[]{0.f, 0.f, 1.f, 0.f, 0.f, 1.f, 1.f, 1.f};
        unsigned inds[]{0, 1, 2, 3};

        auto &rvbo = re.get_buffer(rectvbo);
        auto &rebo = re.get_buffer(rectebo);

        rvbo.push_data(verts, sizeof(verts));
        rebo.push_data(inds, sizeof(inds));
    }

    auto orthoproj = glm::ortho(-100.f, 100.f, -100.f, 100.f, 0.01f, 100.f);

    while (!window->should_close()) {
        float time = glfwGetTime();
        glfwPollEvents();
        glClearColor(cc.x, cc.y, cc.z, 1.);
        window->clear_framebuffer();
        eng::Engine::instance().controller()->update();
        cam.update();

        pbr.use();
        pbr.set("time", (float)glfwGetTime());
        pbr_ubo.set("p", cam.perspective_matrix());
        pbr_ubo.set("v", cam.view_matrix());
        pbr_ubo.set("cam_dir", cam.forward_vec());
        pbr_ubo.set("cam_pos", cam.position());
        pbr_ubo.bind(2);
        re.get_vao(kv).bind();
        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, re.get_buffer(kc).descriptor.handle);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, re.get_buffer(kt).descriptor.handle);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, re.get_buffer(km).descriptor.handle);
        glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, 0, 3, 0);

        auto n       = glm::vec3{0, 0, 1};
        auto f       = cam.forward_vec();
        glm::vec3 yc = glm::normalize(cam.position());
        auto r       = glm::normalize(glm::cross(cam.up, yc));
        auto u       = glm::normalize(glm::cross(yc, r));
        glm::mat4 rotmat{1.f};
        rotmat[0] = glm::vec4{r, 0.f};
        rotmat[1] = glm::vec4{u, 0.f};
        rotmat[2] = glm::vec4{yc, 0.f};

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        vol.use();
        vol.set("ortho", orthoproj);
        vol.set("projection", cam.perspective_matrix());
        vol.set("time", time);
        vol.set("view", cam.view_matrix());
        vol.set("cam_view", cam.forward_vec());
        vol.set("cam_pos", cam.position());
        vol.set("model", model);
        vol.set("rotmat", rotmat);
        re.get_vao(rectvao).bind();
        glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_INT, 0);
        glDisable(GL_BLEND);

        eng::Engine::instance().gui_->draw();
        window->swap_buffers();
    }

    return 0;
}
