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
    pipebuilder.add_stage(sh.get_program(), pipe.vao, DRAW_CMD::MULTI_DRAW_ELEMENTS_INDIRECT, true);
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

    glm::vec3 lpos{1.f}, lcol{1.f};
    float attenuation{1.f};
    bool use_pbr = 1;

    glm::vec3 cc{0.3f};
    Camera::LensSettings lens = cam.lens;
    engine.gui_->add_draw([&] {
        ImGui::SliderFloat3("light position", &lpos.x, -3.f, 3.f);
        ImGui::ColorEdit3("light color", &lcol.x);
        ImGui::ColorEdit3("Background color", &cc.x);
        ImGui::SliderFloat("light attenuation", &attenuation, 0.1f, 1000.f);
        ImGui::Checkbox("Use physically-based rendering", &use_pbr);
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

        sh.use();
        sh.set("model", mat_model);
        sh.set("view", cam.view_matrix());
        sh.set("projection", cam.perspective_matrix());
        sh.set("cam_dir", cam.forward_vec());
        sh.set("cam_pos", cam.position());
        sh.set("lpos", lpos);
        sh.set("lcol", lcol);
        sh.set("attenuation", attenuation);
        sh.set("use_pbr", int{use_pbr});

        re.render();

        eng::Engine::instance().gui_->draw();
        window->swap_buffers();
    }

    return 0;
}
