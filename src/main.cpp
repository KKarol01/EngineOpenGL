#include <filesystem>
#include <ranges>
#include <array>
#include <memory>
#include <numeric>

#include "engine/engine.hpp"
#include "engine/wrappers/include_all.hpp"
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
#include "engine/renderer/pipeline.hpp"
#include "engine/renderer/renderer.hpp"
#include "engine/renderer/pipeline_builder.hpp"

int main() {
    eng::Engine::initialise({Window{"window", 1920, 1080}});
    auto &engine = eng::Engine::instance();
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    const auto window = engine.window();

    Shader sh{"indirect"};
    PBRModelImporter imp{"katana", aiProcess_FlipUVs | aiProcess_Triangulate | aiProcess_CalcTangentSpace};
    auto katana = imp.import_model();

    auto &re    = *engine.renderer_.get();
    auto pipeid = re.create_pipeline();
    auto &pipe  = re.get_pipeline(pipeid);

    PipelineBuilder pipebuilder{pipeid};
    pipebuilder.begin_new_phase();
    pipebuilder.add_stage(&sh, pipe.vao, DRAW_CMD::MULTI_DRAW_ELEMENTS_INDIRECT, true);
    pipebuilder.configure_vao([&](GLVao &vao) {
        vao.configure_binding(0, pipe.geometry_vertices, 14 * 4, 0);
        vao.configure_ebo(pipe.geometry_indices);

        engine.renderer_->get_buffer(pipe.geometry_vertices)
            .descriptor.on_handle_change.connect(
                [&](uint32_t new_handle) { vao.configure_binding(0, new_handle, 14 * 4, 0); });
        engine.renderer_->get_buffer(pipe.geometry_indices)
            .descriptor.on_handle_change.connect([&](uint32_t new_handle) { vao.configure_ebo(new_handle); });

        vao.configure_attr(GL_FLOAT, 4, {{0, 0, 3}, {1, 0, 3}, {2, 0, 3}, {3, 0, 3}, {4, 0, 2}});
    });
    pipebuilder.build();
    pipe.allocate_model(katana);
    pipe.create_instance(katana.id);

    Camera cam;
    glm::mat4 mat_model = glm::scale(glm::mat4{1.f}, glm::vec3{5.f});

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
    glm::vec3 cc{0.3f};
    Camera::LensSettings lens = cam.lens;
    engine.gui_->add_draw([&] {
        ImGui::SliderFloat3("light position", (float *)(ubo_map + 224), -3.f, 3.f);
        ImGui::ColorEdit3("light color", (float *)(ubo_map + 240));
        ImGui::ColorEdit3("Background color", &cc.x);
        ImGui::SliderFloat("light attenuation", (float *)(ubo_map + 256), 0.1f, 1000.f);
        ImGui::Checkbox("Use physically-based rendering", (bool *)(ubo_map + 260));
        if (ImGui::Button("recompile PBR shader")) { sh.recompile(); }

        if (ImGui::CollapsingHeader("Camera lens settings")) {
            ImGui::SliderFloat("FoV", &lens.fovydeg, 0.f, 90.f);
            cam.adjust_lens(lens);
        }
    });

    while (!window->should_close()) {
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
        glBindBufferRange(GL_UNIFORM_BUFFER, 2, ubo.descriptor.handle, 0, sizeof(UBO_CLIENTDATA));

        re.render();

        eng::Engine::instance().gui_->draw();
        window->swap_buffers();
    }

    return 0;
}
