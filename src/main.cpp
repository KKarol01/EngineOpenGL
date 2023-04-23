﻿#include <filesystem>
#include <ranges>
#include <array>
#include <memory>
#include <numeric>

#include "engine/engine.hpp"
#include "engine/camera/camera.hpp"

#include "engine/controller/keyboard/keyboard.hpp"
#include "engine/types/types.hpp"

#include <GLFW/glfw3.h>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <stb_image.h>
#include <imgui/imgui.h>
#include <imgui/imgui_stdlib.h>
#include <glm/gtx/euler_angles.hpp>
#include "engine/renderer/renderer.hpp"

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

    engine.cam = new Camera{};
    engine.cam->set_position(glm::vec3{1.f, 3.f, 5.f});
    Engine::instance().window()->set_clear_flags(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glClearColor(.25f, .25f, .25f, 0.f);

    Renderer r;

    std::vector<float> verts{
        0.f, 0.f, 0.f,
        1.f, 0.f, 0.f,
        0.f, 1.f, 0.f,
        1.f, 1.f, 0.f,
    };
    std::vector<unsigned> inds {0, 1, 2, 2, 1, 3};

    auto prog = r.empty_program();
    *prog = ShaderProgram{"a"};
    
    MaterialPass* forward_untextured_pass = r.empty_material_pass();
    forward_untextured_pass->pipelines[PipelinePass::Forward] = prog;

    Material def_mat;
    def_mat.pass = forward_untextured_pass;

    Mesh mesh_rectangle {
        .id = 1,
        .material = &def_mat,
        .transform = glm::translate(glm::mat4{1.f}, glm::vec3{3.f, 0.f, 0.f}),
        .vertices = verts,
        .indices = inds
    };
    Mesh mesh_triangle {
        .id = 2,
        .material = &def_mat,
        .vertices = verts,
        .indices = {0, 1, 2}
    };

    Object rectangle {
        .id = 1,
        .meshes = {mesh_rectangle}
    };
    Object triangle {
        .id = 2,
        .meshes = {mesh_triangle}
    };
    
    r.register_object(&triangle);
    r.register_object(&rectangle);
    r.render();

    while (!window->should_close()) {
        float time = glfwGetTime();
        glfwPollEvents();

        eng::Engine::instance().controller()->update();
        engine.cam->update();

        window->clear_framebuffer();
        r.render();

        window->swap_buffers();
    }

    return 0;
}
