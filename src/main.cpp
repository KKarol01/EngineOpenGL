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

int main() {
    eng::Engine::initialise("window", 1920, 1080);
    auto &engine      = eng::Engine::instance();
    const auto window = engine.window();
    auto cam          = Camera{};

    auto &r    = *engine.renderer_.get();
    auto vaoid = r.vaos.emplace();
    auto vboid = r.buffers.emplace(eng::GLBufferDescriptor{GL_DYNAMIC_STORAGE_BIT});
    auto eboid = r.buffers.emplace(eng::GLBufferDescriptor{GL_DYNAMIC_STORAGE_BIT});
    auto &vbo  = r.buffers[vboid];
    auto &ebo  = r.buffers[eboid];
    auto &vao  = r.vaos[vaoid];

    float vbodata[]{0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 1.f, 0.f, 1.f, 1.f, 0.f};
    unsigned ebodata[]{0, 1, 2, 2, 1, 3};
    vao.configure_binding(0, vboid, 12, 0);
    vao.configure_ebo(eboid);
    vao.configure_attributes({eng::GLVaoAttributeDescriptor{0, 0, 3, 0}});

    vbo.push_data(vbodata, sizeof(vbodata));
    ebo.push_data(ebodata, sizeof(ebodata));

    using namespace eng;

    auto rectid = r.programs[ShaderProgram{"rect"}];
    auto gjkid  = r.programs[ShaderProgram{"gjk"}];

    auto rectuboid = r.buffers[GLBuffer{}];

    Pipeline pp;
    PipelineStage pps1{
        .vao = vaoid, .program = rectid, .draw_cmd = std::make_shared<DrawElementsCMD>(eboid), .bufferbinders = {}};
    pp.stages = {pps1};
    auto ppid = r.pipelines[pp];

    auto linebufid = r.buffers[GLBuffer{}];
    auto &linebuf  = r.buffers[linebufid];

    glm::vec3 ls[2]{
        glm::vec3{0., 0., 100.},
        glm::vec3{0., 0., -100.},
    };
    linebuf.push_data(ls, 24);
    auto linevaoid = r.vaos[GLVao{}];
    auto &linevao  = r.vaos[linevaoid];
    linevao.configure_binding(0, linebufid, 12, 0);
    linevao.configure_attributes({
        eng::GLVaoAttributeDescriptor{0, 0, 3, 0},
    });

    glm::vec3 clear_color{.25f};
    glm::vec4 plane_rot_scale{0.f, 0.f, 0.f, 1.f};
    glm::vec4 cube_rot_scale{0.f, 0.f, 0.f, 1.f};
    glm::vec3 plane_tr{0.f}, cube_tr{0.f};
    glm::mat4 plane_model{1.f}, cube_model{1.f};
    engine.gui_->add_draw([&] {
        auto cam_dir = cam.forward_vec();
        auto cam_pos = cam.position();
        ImGui::SliderFloat3("cam_dir", &cam_dir.x, 0., 1.);
        ImGui::SliderFloat3("cam_position", &cam_pos.x, 0., 1.);
        if (ImGui::Button("recompile")) { r.programs[rectid].recompile(); }
        if (ImGui::Button("recompile1")) { r.programs[gjkid].recompile(); }

        ImGui::SliderFloat3("plane transform", &plane_tr.x, -5.f, 5.f);
        ImGui::SliderFloat3("cube transform", &cube_tr.x, -5.f, 5.f);
        ImGui::SliderFloat4("plane rot scale", &plane_rot_scale.x, -1.f, 1.f);
        ImGui::SliderFloat4("cube rot scale", &cube_rot_scale.x, -1.f, 1.f);

        plane_model = glm::translate(glm::eulerAngleXYZ(plane_rot_scale.x, plane_rot_scale.y, plane_rot_scale.z)
                                         * glm::scale(glm::mat4{1.f}, glm::vec3{plane_rot_scale.w}),
                                     plane_tr);
        cube_model  = glm::translate(glm::eulerAngleXYZ(cube_rot_scale.x, cube_rot_scale.y, cube_rot_scale.z)
                                        * glm::scale(glm::mat4{1.f}, glm::vec3{cube_rot_scale.w}),
                                    cube_tr);
    });

    ModelImporter imp;
    auto gjk_test_cube  = imp.import_model("3dmodels/simple_shapes/cube.obj", aiProcess_Triangulate);
    auto gjk_test_plane = imp.import_model("3dmodels/simple_shapes/plane.obj", aiProcess_Triangulate);
    auto gjkvaoid       = r.vaos[GLVao{}];
    auto &gjkvao        = r.vaos[gjkvaoid];
    {
        auto gjkvboid = r.buffers[GLBuffer{}];
        auto gjkeboid = r.buffers[GLBuffer{}];
        auto &gjkvbo  = r.buffers[gjkvboid];
        auto &gjkebo  = r.buffers[gjkeboid];

        gjkvbo.push_data(gjk_test_cube.vertices.data(), gjk_test_cube.vertices.size() * 4);
        gjkebo.push_data(gjk_test_cube.indices.data(), gjk_test_cube.indices.size() * 4);
        gjkvbo.push_data(gjk_test_plane.vertices.data(), gjk_test_plane.vertices.size() * 4);
        gjkebo.push_data(gjk_test_plane.indices.data(), gjk_test_plane.indices.size() * 4);

        gjkvao.configure_binding(0, gjkvboid, gjk_test_cube.meshes[0].stride * 4, 0);
        gjkvao.configure_attributes({GLVaoAttributeDescriptor{0, 0, 3, 0}, GLVaoAttributeDescriptor{1, 0, 3, 3}});
        gjkvao.configure_ebo(gjkeboid);
    }
    GJK gjk;
    glEnable(GL_DEPTH_TEST);
    while (!window->should_close()) {
        float time = glfwGetTime();
        glfwPollEvents();
        glClearColor(clear_color.x, clear_color.y, clear_color.z, 1.);
        Engine::instance().window()->set_clear_flags(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        Engine::instance().window()->clear_framebuffer();
        eng::Engine::instance().controller()->update();
        cam.update();

        r.vaos[gjkvaoid].bind();
        r.programs[gjkid].use();
        r.programs[gjkid].set("model", cube_model);
        r.programs[gjkid].set("proj", cam.perspective_matrix());
        r.programs[gjkid].set("view", cam.view_matrix());

        glEnable(GL_STENCIL_TEST);
        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
        glStencilFunc(GL_ALWAYS, 1, 0xFF);
        glStencilMask(0xFF);
        r.programs[gjkid].set("silhouette", 0);
        glDrawElementsBaseVertex(GL_TRIANGLES, gjk_test_cube.meshes[0].index_count, GL_UNSIGNED_INT, 0, 0);
        glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
        glDisable(GL_DEPTH_TEST);
        r.programs[gjkid].set("silhouette", 1);
        cube_model = glm::scale(glm::mat4{1.f}, glm::vec3{1.005f});
        r.programs[gjkid].set("model", cube_model);
        glDrawElementsBaseVertex(GL_TRIANGLES, gjk_test_cube.meshes[0].index_count, GL_UNSIGNED_INT, 0, 0);
       
        glStencilMask(0xFF);
        glStencilFunc(GL_EQUAL, 0, 0xFF);
        r.programs[rectid].use();
        r.programs[rectid].set("model", glm::mat4{1.f});
        r.programs[rectid].set("proj", cam.perspective_matrix());
        r.programs[rectid].set("view", cam.view_matrix());
        r.vaos[linevaoid].bind();
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        glDrawArraysInstancedBaseInstance(GL_LINES, 0, 2, 101, 0);
        r.programs[rectid].set("model", glm::rotate(glm::mat4{1.f}, glm::radians(90.f), glm::vec3{0.f, 1.f, 0.f}));
        glDrawArraysInstancedBaseInstance(GL_LINES, 0, 2, 101, 102);
        
        glDisable(GL_BLEND);
        glDisable(GL_STENCIL_TEST);
        glEnable(GL_DEPTH_TEST);

        //  r.programs[gjkid].set("silhouette", 0);
        //  r.programs[gjkid].set("model", plane_model);
        //  glDrawElementsBaseVertex(GL_TRIANGLES,
        //                           gjk_test_plane.meshes[0].index_count,
        //                           GL_UNSIGNED_INT,
        //                           0,
        //                           gjk_test_cube.meshes[0].vertex_count);
        //
        std::cout << gjk.intersect(gjk_test_cube, gjk_test_plane, cube_model, plane_model) << '\n';

        eng::Engine::instance().gui_->draw();
        window->swap_buffers();
    }

    return 0;
}
