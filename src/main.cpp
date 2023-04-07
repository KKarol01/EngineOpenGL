#include <filesystem>
#include <ranges>
#include <array>
#include <memory>
#include <numeric>

#include "engine/engine.hpp"
#include "engine/camera/camera.hpp"

#include "engine/controller/keyboard/keyboard.hpp"
#include "engine/types/types.hpp"

#include "3dcalc/3dcalc.hpp"

#include <GLFW/glfw3.h>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <stb_image.h>
#include <imgui/imgui.h>
#include <imgui/imgui_stdlib.h>
#include <glm/gtx/euler_angles.hpp>

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
    using namespace eng;
    // auto rect_program    = engine.renderer_->programs.emplace("rect");
    /*auto gjk_test_cube  = imp.import_model("3dmodels/simple_shapes/cube.obj", aiProcess_Triangulate);
     auto gjk_test_plane = imp.import_model("3dmodels/simple_shapes/plane.obj", aiProcess_Triangulate);*/

    /*engine.cam = new Camera{};
    Graph3D graph;
    engine.cam->set_position(glm::vec3{1.f,3.f,5.f});
    Engine::instance().window()->set_clear_flags(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glClearColor(.25f, .25f, .25f, 0.f);*/
    while (!window->should_close()) {
        //float time = glfwGetTime();
        //glfwPollEvents();

        //eng::Engine::instance().controller()->update();
        //engine.cam->update();

        //glEnable(GL_DEPTH_TEST);
        //glEnable(GL_BLEND);
        //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        //glViewport(0, 0, 1920.f * .75f, 1080.f* .75f);
        //glBindFramebuffer(GL_FRAMEBUFFER, graph.fbo);
        //window->clear_framebuffer();
        //graph.render();
        //engine.renderer_->render_frame();

        //window->adjust_glviewport();
        //glBindFramebuffer(GL_FRAMEBUFFER, 0);
        //window->clear_framebuffer();
        //eng::Engine::instance().gui_->draw();
        //window->swap_buffers();
    }

    return 0;
}
