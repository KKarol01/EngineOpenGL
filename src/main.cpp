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

int main() {
    Engine::initialize(Window{"window", 1920, 1080});
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    const auto window = Engine::window();

    struct RD {
        Shader *sh{nullptr};
        std::vector<SHADERDATA> sh_datas;
        VAO vao;
    };

    class A : public SystemBase {
        std::map<Shader *, SortedVectorUnique<EntityID>> entities;
        std::map<EntityID, SortedVectorUnique<EntityID> *> ent_vec;

      public:
        A() : SystemBase() { set_component_family<RD>(Engine::ecs()); }

        void update_entity(EntityID id) final {
            const auto &e       = Engine::ecs()->get_entity(id);
            const auto accepted = entity_compatible(e);

            if (!accepted && !ent_vec.contains(id)) {
                return;
            } else if (!accepted) {
                ent_vec.at(id)->remove(id);
                ent_vec.erase(id);
                return;
            }

            const auto &rd = Engine::ecs()->get_component<RD>(id);

            if (ent_vec.contains(id)) { ent_vec.at(id)->remove(id); }
            if (rd.sh == nullptr) return;

            entities[rd.sh].insert(id);
            ent_vec[id] = &entities.at(rd.sh);
        }
        void update() final {
            for (const auto &[sh, ids] : entities) {
                sh->use();
                for (const auto id : ids.data()) {
                    const auto &comp = Engine::ecs()->get_component<RD>(id);
                    comp.vao.bind();
                    sh->feed_uniforms(comp.sh_datas);
                    comp.vao.draw();
                }
            }
        }
    };

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

    auto &ecs = *Engine::ecs();
    auto ent  = ecs.create_entity();
    ecs.register_component<RD>();
    A system;
    system.update_entity(ent);
    auto &comp = ecs.add_component<RD>(ent);
    system.update_entity(ent);
    comp.sh       = Engine::shader_manager()->get_shader("default");
    comp.sh_datas = {{"color", glm::vec3{1.f, 0.f, 0.f}}};
    comp.vao      = std::move(triangle);
    system.update_entity(ent);


    while (!window->should_close()) {
        glfwPollEvents();
        Engine::update();
        c.update();

        skybox.uniforms.at(0).value = c.view_matrix();
        skybox.uniforms.at(1).value = c.perspective_matrix();


        Engine::render_frame();
        system.update();
        Engine::window()->swap_buffers();
    }

    Engine::terminate();
    return 0;
}
