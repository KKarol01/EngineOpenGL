

#include "engine/engine.hpp"
#include "engine/wrappers/include_all.hpp"
#include "engine/camera/camera.hpp"
#include "engine/subsystems/renderer/components.hpp"
#include "engine/subsystems/ecs/ecs.hpp"

#include <GLFW/glfw3.h>

int main() {
    Engine::initialise({Window{"window", 1920, 1080}});
    auto &engine = Engine::instance();
   // glEnable(GL_DEPTH_TEST);
   // glEnable(GL_CULL_FACE);

    const auto window = engine.window();

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

    auto ecs = Engine::instance().ecs();
    for (auto i = 0u; i < 1u; ++i) {

        VAO triangle;
        {
            std::vector<glm::vec2> vertices{
                {0.f, 0.f},
                {1.f, 0.f},
                {0.f, 1.f},
            };
            std::vector<unsigned> indices{0, 1, 2};
            triangle.insert_vbo(BUFFEROBJECT{0, vertices});
            triangle.insert_ebo(3, BUFFEROBJECT{0, indices});
            triangle.configure(2, GL_FLOAT, 4, {ATTRSAMEFORMAT{0, 0}});
            triangle.draw = [] { glDrawArrays(GL_TRIANGLES, 0, 3); };
        }
        auto e1 = ecs->create_entity();
        RenderData rd1{.vao      = std::move(triangle),
                       .sh       = Engine::instance().shader_manager()->get_shader("default"),
                       .sh_datas = {{"color", glm::vec3{1.f, 0.f, 0.f}}}};
        ecs->add_component(e1, &rd1);
    }

    while (!window->should_close()) {
        glfwPollEvents();
        c.update();
        Engine::instance().update();
    }

    return 0;
}
