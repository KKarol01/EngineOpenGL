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

#include "engine/engine.hpp"
#include "engine/wrappers/include_all.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <stb_image.h>

#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

#include "engine/ecs/include.hpp"
#include "engine/ecs/sorted_vec.hpp"

int main() {
    Engine::initialize(Window{"window", 1920, 1080});
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    const auto window = Engine::window();

    struct eRenderData {
        VAO vao;
        Shader *sh{nullptr};
        std::vector<SHADERDATA> sh_data;

        eRenderData() {}
        eRenderData(const eRenderData &d) {}
    };

    class RenderSystem : public SystemBase {
        std::map<Shader *, SortedVectorUnique<EntityID>> entities;

      public:
        RenderSystem() { set_component_family<eRenderData>(Engine::ecs()->components_mgr()); }

        void insert_entity(const Entity &e) final {
            auto sh = Engine::ecs()->get_component<eRenderData>(e.id);
            if (sh) entities[sh.value()->sh].insert(e.id);
        }
        void remove_entity(const Entity &e) final {
            auto sh = Engine::ecs()->get_component<eRenderData>(e.id);

            if (sh && entities.contains(sh.value()->sh)) entities[sh.value()->sh].remove(e.id);
        }
        void update() final {
            for (const auto &e : entities) {
                e.first->use();
                auto &at = entities.at(e.first);
                for (auto it = at.begin(); it != at.end(); ++it) {
                    auto data = Engine::ecs()->get_component<eRenderData>(*it).value();

                    data->vao.bind();
                    e.first->feed_uniforms(data->sh_data);
                    data->vao.draw();
                }
            }
        }
    };

    SortedVectorUnique<int> ints;
    ints.emplace(1);
    ints.emplace(1);
    ints.emplace(3);
    ints.emplace(2);
    ints.emplace(2);
    ints.emplace(0);
    ints.emplace(1);

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
    std::vector<glm::vec2> verts{{0.f, 0.f}, {1.f, 0.f}, {0.f, 1.f}};
    std::vector<unsigned> inds{0, 1, 2};

    VAO triangle;
    triangle.insert_vbo(BUFFEROBJECT{0, verts});
    triangle.insert_ebo(3, {0, inds});
    triangle.configure(2, GL_FLOAT, 4, {ATTRSAMEFORMAT{0, 0}});
    triangle.set_draw_func(glDrawElements, GL_TRIANGLES, inds.size(), GL_UNSIGNED_INT, nullptr);

    auto &ecs = *Engine::ecs();
    ecs.register_component<eRenderData>();
    ecs.add_system<RenderSystem>();
    auto eid  = ecs.create_entity();
    auto comp = ecs.add_component<eRenderData>(eid,
                                               std::move(triangle),
                                               Engine::shader_manager()->get_shader("default"),
                                               std::vector<SHADERDATA>{{{"color", glm::vec3{1.f, 0.f, 0.f}}}});

    Scene scene_;
    Engine::set_scene(&scene_);
    Engine::renderer_->register_material("sky", "sky");
    RenderData skybox;
    skybox_vao.set_draw_func(glDrawArrays, GL_TRIANGLES, 0, 36);
    skybox.vao = std::move(skybox_vao);
    skybox.textures.emplace_back(1, std::move(skybox_tex));
    skybox.uniforms = {{"view", glm::mat4{1.f}}, {"projection", glm::mat4{1.f}}};
    Engine::renderer_->add_to_draw_list("sky", &skybox);

    Camera c;

    while (!window->should_close()) {
        glfwPollEvents();
        Engine::update();
        c.update();

        skybox.uniforms.at(0).value = c.view_matrix();
        skybox.uniforms.at(1).value = c.perspective_matrix();

        Engine::render_frame();
    }

    Engine::terminate();
    return 0;
}
