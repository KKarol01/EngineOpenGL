#include <filesystem>
#include <ranges>
#include <array>
#include <memory>
#include <numeric>

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
    Engine::instance().window()->set_clear_flags(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glClearColor(.25f, .25f, .25f, 0.f);

    Renderer r;

    auto prog = &(*r.empty_program() = ShaderProgram{"a"});

    MaterialPass *forward_untextured_pass                     = r.empty_material_pass();
    forward_untextured_pass->pipelines[PipelinePass::Forward] = prog;

    Material def_mat;
    def_mat.pass = forward_untextured_pass;
    ShaderData object_attributes{.data = {}, .bytes_per_instance = 16};

    def_mat.data = &object_attributes;

    {
        Assimp::Importer i;
        auto scene = i.ReadFile("3dmodels/cube.obj", aiProcess_Triangulate);

        std::vector<Mesh> meshes;

        std::function<void(const aiScene *scene, const aiNode *node)> parse_scene;
        parse_scene = [&](const aiScene *scene, const aiNode *node) {
            for (auto i = 0u; i < node->mNumMeshes; ++i) {
                auto mesh = scene->mMeshes[node->mMeshes[i]];

                std::vector<float> mesh_vertices;
                std::vector<unsigned> mesh_indices;

                for (auto j = 0u; j < mesh->mNumVertices; ++j) {
                    auto &v = mesh->mVertices[j];
                    
                    mesh_vertices.push_back(v.x);
                    mesh_vertices.push_back(v.y);
                    mesh_vertices.push_back(v.z);
                    mesh_vertices.push_back(mesh->mNormals[i].x);
                    mesh_vertices.push_back(mesh->mNormals[i].y);
                    mesh_vertices.push_back(mesh->mNormals[i].z);

                    auto n = glm::mix(glm::vec3{0.f}, glm::vec3{1.f}, (float)j/(float)mesh->mNumVertices);
                    def_mat.data->data.push_back(mesh->mNormals[j].x);
                    def_mat.data->data.push_back(mesh->mNormals[j].y);
                    def_mat.data->data.push_back(mesh->mNormals[j].z);
                }

                for (auto j = 0u; j < mesh->mNumFaces; ++j) {
                    auto &face = mesh->mFaces[j];

                    assert((face.mNumIndices == 3 && "Accepting only triangular faces"));

                    mesh_indices.push_back(face.mIndices[0]);
                    mesh_indices.push_back(face.mIndices[1]);
                    mesh_indices.push_back(face.mIndices[2]);
                }

                Material mat{.pass = def_mat.pass};

                meshes.push_back(Mesh{.id        = (uint32_t)meshes.size() + 1,
                                      .material  = &def_mat,
                                      .transform = glm::mat4{1.f},
                                      .vertices  = mesh_vertices,
                                      .indices   = mesh_indices});
            }

            for (auto i = 0u; i < node->mNumChildren; ++i) { parse_scene(scene, node->mChildren[i]); }
        };

        parse_scene(scene, scene->mRootNode);

        uint32_t object_counter = 1;
        for (const auto &m : meshes) {
            Object o{.id = object_counter++, .meshes = {m}};
            r.register_object(&o);
        }
    }

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
