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
#include "engine/allocators/idallocator.hpp"
#include "engine/physics/gjk.hpp"

const auto make_model = [](glm::vec3 t, glm::vec3 r, glm::vec3 s) -> glm::mat4 {
    static constexpr auto I = glm::mat4{1.f};
    const auto T            = glm::translate(I, t);
    const auto R            = glm::eulerAngleXYZ(r.x, r.y, r.z);
    const auto S            = glm::scale(I, s);
    return T * R * S;
};

int main() {
    eng::Engine::initialise("window", 1920, 1080);
    auto &engine      = eng::Engine::instance();
    const auto window = engine.window();
    auto cam          = Camera{};
    using namespace eng;

    auto default_program = engine.renderer_->programs.emplace("cube");
    auto rect_program    = engine.renderer_->programs.emplace("rect");

    ModelImporter imp;
    auto gjk_test_cube  = imp.import_model("3dmodels/simple_shapes/cube.obj", aiProcess_Triangulate);
    auto gjk_test_plane = imp.import_model("3dmodels/simple_shapes/plane.obj", aiProcess_Triangulate);

    auto pp2 = engine.renderer_->pipelines.emplace();
    PipelineStage pp2s1;
    auto pp2s1vao = engine.renderer_->vaos.emplace();
    pp2s1vao->configure_ebo(pp2->ebo);
    pp2s1vao->configure_binding(0, pp2->vbo, 8, 0);
    pp2s1vao->configure_attributes(GL_FORMAT_FLOAT, 4, {GLVaoAttributeSameType{0, 0, 2}});

    engine.gui_->add_draw([&]() {
        if (ImGui::Begin("test")) { 
            if(ImGui::Button("Recompile shader")) {
                engine.renderer_->programs[pp2s1.program].recompile();
            }
            
            static char buff[512];

            
            if(ImGui::InputTextMultiline("code", buff, 512, ImVec2(0,0))) {
                printf("char");
            }
        }

        ImGui::End();
    });

    {
        uint32_t indices[4]{0, 2, 1, 3};
        glm::vec2 vertices[4]{
            {0.f, 0.f},
            {1.f, 0.f},
            {0.f, 1.f},
            {1.f, 1.f},
        };

        engine.renderer_->buffers[pp2->vbo].push_data(vertices, sizeof(vertices));
        engine.renderer_->buffers[pp2->ebo].push_data(indices, sizeof(indices));
    }
    pp2s1.vao            = pp2s1vao;
    pp2s1.program        = default_program.id;
    pp2s1.draw_cmd       = std::make_shared<DrawElementsInstancedCMD>(pp2->ebo, 1000000);
    pp2s1.on_stage_start = [&] {
        default_program->set("model", glm::mat4{1.f});
        default_program->set("view", cam.view_matrix());
        default_program->set("projection", cam.perspective_matrix());
        default_program->set("cam_pos", cam.position());
    };

    pp2->stages.push_back(std::move(pp2s1));

    while (!window->should_close()) {
        float time = glfwGetTime();
        glfwPollEvents();
        glClearColor(.25f, .25f, .25f, 1.);
        Engine::instance().window()->set_clear_flags(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        Engine::instance().window()->clear_framebuffer();
        eng::Engine::instance().controller()->update();
        cam.update();

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_DEPTH_TEST);
        engine.renderer_->render();

        eng::Engine::instance().gui_->draw();
        window->swap_buffers();
    }

    return 0;
}
