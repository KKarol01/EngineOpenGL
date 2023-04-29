#include <engine/gpu/shaderprogram/shader_dec.hpp>
#include <filesystem>
#include <ranges>
#include <array>
#include <memory>
#include <numeric>
#include <initializer_list>

#include <engine/engine.hpp>
#include <engine/camera/camera.hpp>
#include <engine/controller/keyboard/keyboard.hpp>
#include <engine/types/types.hpp>
#include <engine/renderer/renderer.hpp>

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

    engine.cam = new Camera{};
    engine.cam->set_position(glm::vec3{1.f, 3.f, 5.f});
    Engine::instance().window()->set_clear_flags(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT
                                                 | GL_STENCIL_BUFFER_BIT);
    glClearColor(.25f, .25f, .25f, 0.f);

    Renderer r;

    // auto hprog            = r.empty_program();
    // *r.get_program(hprog) = ShaderProgram{"a"};

    // Handle<MaterialPass> forward_untextured_pass = r.empty_material_pass();
    // r.get_material_pass(forward_untextured_pass)->pipelines[PipelinePass::Forward]
    //     = r.get_program(hprog);

    // Material def_mat;
    // def_mat.pass = r.get_material_pass(forward_untextured_pass);
    // ShaderData object_attributes{.data = {}, .bytes_per_instance = 16};

    // def_mat.data = &object_attributes;

    // {
    //     Assimp::Importer i;
    //     auto scene = i.ReadFile("3dmodels/bust/scene.gltf", aiProcess_Triangulate);

    //     std::vector<Mesh> meshes;

    //     std::function<void(const aiScene *scene, const aiNode *node)> parse_scene;
    //     parse_scene = [&](const aiScene *scene, const aiNode *node) {
    //         for (auto i = 0u; i < node->mNumMeshes; ++i) {
    //             auto mesh = scene->mMeshes[node->mMeshes[i]];

    //             std::vector<float> mesh_vertices;
    //             std::vector<unsigned> mesh_indices;

    //             for (auto j = 0u; j < mesh->mNumVertices; ++j) {
    //                 auto &v = mesh->mVertices[j];

    //                 mesh_vertices.push_back(v.x);
    //                 mesh_vertices.push_back(v.y);
    //                 mesh_vertices.push_back(v.z);
    //                 mesh_vertices.push_back(mesh->mNormals[i].x);
    //                 mesh_vertices.push_back(mesh->mNormals[i].y);
    //                 mesh_vertices.push_back(mesh->mNormals[i].z);
    //             }

    //             for (auto j = 0u; j < mesh->mNumFaces; ++j) {
    //                 auto &face = mesh->mFaces[j];

    //                 assert((face.mNumIndices == 3 && "Accepting only triangular faces"));

    //                 mesh_indices.push_back(face.mIndices[0]);
    //                 mesh_indices.push_back(face.mIndices[1]);
    //                 mesh_indices.push_back(face.mIndices[2]);
    //             }

    //             Material mat{.pass = def_mat.pass};

    //             meshes.push_back(Mesh{.id        = (uint32_t)meshes.size() + 1,
    //                                   .material  = &def_mat,
    //                                   .transform = glm::mat4{1.f},
    //                                   .vertices  = mesh_vertices,
    //                                   .indices   = mesh_indices});
    //         }

    //         for (auto i = 0u; i < node->mNumChildren; ++i) {
    //             parse_scene(scene, node->mChildren[i]);
    //         }
    //     };

    //     parse_scene(scene, scene->mRootNode);

    //     const auto swap = [](int a, int b, auto &v) {
    //         auto t = v[a];
    //         v[a]   = v[b];
    //         v[b]   = t;
    //     };

    //     uint32_t object_counter = 1;
    //     for (auto &m : meshes) {
    //         m.transform = glm::translate(glm::mat4{1.f}, glm::vec3{0.f, 0.f, -1.f});
    //     }
    //     Object o{.id = object_counter++, .meshes = meshes};

    //     for (int i = 0; i < 3; ++i) {
    //         o.meshes = meshes;
    //         o.id++;
    //         for (auto &m : o.meshes) {
    //             m.transform = glm::rotate(
    //                 m.transform, 3.14f / 2.f * ((float)i - 1.f), glm::vec3{0.f, 1.f, 0.f});
    //         }

    //         r.register_object(&o);
    //     }
    // }

    //   r.render();
    ShaderProgram prog{"a"};
    Material m{.passes = {{RenderPass::Forward, &prog}}};

    {
        // clang-format off
        std::array<std::initializer_list<float>, 3> meshes {
            {
                {
                    0.f, 0.f, 0.f,
                    1.f, 0.f, 0.f,
                    0.f, 1.f, 0.f,
                },

                {
                    0.f, 0.f, 0.f,
                    1.f, 0.f, 0.f,
                    0.f, 1.f, 0.f,
                    1.f, 1.f, 0.f,
                },

                {
                   -1.f, 0.f, 0.f,
                    1.f, 0.f, 0.f,
                    0.f, 1.f, 0.f,
                },

            }
        };

        std::array<std::initializer_list<unsigned>, 3> indices {
            {
                {0, 1, 2},
                {0, 1, 2, 2, 1, 3},
                {0, 1, 2}
            }
        };
        // clang-format on

        Object o;
        for (int i = 0; i < 3; ++i) {
            o.meshes.push_back(Mesh{.material = &m, .vertices = meshes[i], .indices = indices[i]});
        }

        std::vector<Object> objects;
        for (int i = 0; i < 3; ++i) {
            objects.push_back(o);
            objects.back().id += i;
        }

        for (auto &o : objects) {
            o.meshes.resize(o.id);
            for (auto &m : o.meshes) {
                m.transform = glm::rotate(glm::mat4{1.f},
                                          3.14f / 2.0f * (float)(o.id - 2),
                                          glm::vec3{0.f, 1.f, 0.f})
                              * glm::translate(glm::mat4{1.f}, glm::vec3{0.f, 0.f, -1.f});
            }

            r.register_object(&o);
        }
    }

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
