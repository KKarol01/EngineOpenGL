#include <iostream>
#include <string>
#include <vector>
#include <array>
#include <tuple>
#include <map>
#include <fstream>
#include <functional>
#include <cstdio>
#include <typeinfo>
#include <typeindex>
#include <chrono>
#include <algorithm>
#include <concepts>
#include <ranges>

#include "engine/engine.hpp"
#include "engine/wrappers/include_all.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <stb_image.h>

#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

#include "engine/ecs/ecs.hpp"
#include "engine/renderer/components.hpp"
#include "timer/timer.hpp"

int main() {
    Engine::initialize(Window{"window", 1920, 1080});
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    const auto window = Engine::window();

    VAO skybox_vao;
    Texture skybox_tex{GL_LINEAR, GL_CLAMP_TO_EDGE};
    {
        float skyboxVertices[]
            = {// positions
               -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,  1.0f,
               -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f,
               1.0f,  -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f, 1.0f,
               1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, -1.0f,
               1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f,
               -1.0f, 1.0f,  -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,
               -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f,
               -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f};
        skybox_vao.insert_vbo(BUFFEROBJECT{0, skyboxVertices, sizeof(skyboxVertices) / 4, 12, 0});
        skybox_vao.configure(3, GL_FLOAT, 4, {ATTRSAMEFORMAT{0, 0}});
        std::string paths[6] = {"textures/right.jpg",
                                "textures/left.jpg",
                                "textures/top.jpg",
                                "textures/bottom.jpg",
                                "textures/front.jpg",
                                "textures/back.jpg"};
        skybox_tex.buildcube(paths, GL_RGB8);
    }

    Camera c;

    auto ecs = Engine::ecs();
    VAO triangle;
    {
        Timer time{"draw func setup"};
        std::vector<glm::vec2> vertices{
            {0.f, 0.f},
            {1.f, 0.f},
            {0.f, 1.f},
        };
        std::vector<unsigned> indices{0, 1, 2};
        triangle.insert_vbo(BUFFEROBJECT{0, vertices});
        triangle.insert_ebo(3, BUFFEROBJECT{0, indices});
        triangle.configure(2, GL_FLOAT, 4, {ATTRSAMEFORMAT{0, 0}});
        time.start();
        triangle.draw = [] { glDrawArrays(GL_TRIANGLES, 0, 3); };
        time.step()("name");
        time.end();
    }
    auto e1 = ecs->create_entity();
    RenderData rd1{.vao      = std::move(triangle),
                   .sh       = Engine::shader_manager()->get_shader("default"),
                   .sh_datas = {{"color", glm::vec3{1.f, 0.f, 0.f}}}};
    ecs->add_component(e1, &rd1);

    while (!window->should_close()) {
        glfwPollEvents();
        Engine::update();
        c.update();

        Engine::render_frame();
    }

    Engine::terminate();
    return 0;
}
