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

    engine.gui_->add_draw([&] {

    });

    auto default_program = engine.renderer_->programs.emplace("cube");
    auto rect_program    = engine.renderer_->programs.emplace("rect");

    ModelImporter imp;
    auto gjk_test_cube  = imp.import_model("3dmodels/simple_shapes/cube.obj", aiProcess_Triangulate);
    auto gjk_test_plane = imp.import_model("3dmodels/simple_shapes/plane.obj", aiProcess_Triangulate);

    // ModelPipelineAdapter ppadapter{{ModelPipelineAdapter::ATTR_POSITION}};
    // auto pp = engine.renderer_->pipelines.emplace(ppadapter);

    // PipelineStage ppstage1;
    // auto ppstage1vao = engine.renderer_->vaos.emplace();
    // ppstage1vao->configure_binding(0, pp->vbo, pp->adapter.stride * 4, 0);
    // ppstage1vao->configure_ebo(pp->ebo);
    // ppstage1vao->configure_attributes(GL_FLOAT, 4, {GLVaoAttributeSameType{0, 0, 3}});
    // ppstage1.vao      = ppstage1vao;
    // ppstage1.program  = default_program.id;
    // ppstage1.draw_cmd = std::make_shared<DrawElementsCMD>(pp->vbo);
    // pp->on_model_add.connect([&](const Model &m) {
    //     static_cast<DrawElementsCMD *>(pp->stages[0].draw_cmd.get())->count = m.indices.size();
    // });
    // pp->stages.push_back(std::move(ppstage1));
    // pp->add_model(gjk_test_cube);

    auto pp2 = engine.renderer_->pipelines.emplace();
    PipelineStage pp2s1, pp2s2;
    auto pp2s1vao = engine.renderer_->vaos.emplace();
    pp2s1vao->configure_binding(0, pp2->vbo, 12, 0);
    pp2s1vao->configure_attributes(GL_FLOAT, 4, {GLVaoAttributeSameType{0, 0, 3}});
    {
        float line_verts[]{0.f, 0.f, -50.f, 0.f, 0.f, 50.f};
        engine.renderer_->buffers[pp2->vbo].push_data(line_verts, sizeof(line_verts));
    }
    pp2s1.vao            = pp2s1vao;
    pp2s1.program        = rect_program.id;
    pp2s1.draw_cmd       = std::make_shared<DrawArraysInstancedBaseInstanceCMD>(2, 101, 0, 0, GL_LINES);
    pp2s2.draw_cmd       = std::make_shared<DrawArraysInstancedBaseInstanceCMD>(2, 101, 0, 102, GL_LINES);
    pp2s1.on_stage_start = [&] {
        rect_program->set("model", glm::mat4{1.f});
        rect_program->set("view", cam.view_matrix());
        rect_program->set("projection", cam.perspective_matrix());
    };
    pp2s2.on_stage_start = [&] {
        static const auto R = glm::rotate(glm::mat4{1.f}, 3.14f / 2.f, glm::vec3{0, 1, 0});
        rect_program->set("model", R);
    };
    pp2->stages.push_back(std::move(pp2s1));
    pp2->stages.push_back(std::move(pp2s2));

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
