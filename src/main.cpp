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

int main() {
    eng::Engine::initialise("window", 1920, 1080);

    auto &engine      = eng::Engine::instance();
    const auto window = engine.window();
    auto cam          = Camera{};
    constexpr float x = 1.f;

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

    auto rectuboid = r.buffers[GLBuffer{}];
    UBO u{{
        {"proj", glm::mat4{1.f}},
        {"view", glm::mat4{1.f}},
        {"pos", glm::vec4{1.f}},
        {"dir", glm::vec4{1.f}},
    }};

    Pipeline pp;
    PipelineStage pps1{.vao           = vaoid,
                       .program       = rectid,
                       .draw_cmd      = std::make_shared<DrawElementsCMD>(eboid),
                       .bufferbinders = {std::make_shared<BufferBasedBinder>(GL_UNIFORM_BUFFER, u.get_bufferid(), 0)}};
    pp.stages = {pps1};
    auto ppid = r.pipelines[pp];

    auto linebufid = r.buffers[GLBuffer{}];
    auto &linebuf  = r.buffers[linebufid];

    glm::vec3 ls[2]{
        glm::vec3{0., 0., 100.},
        glm::vec3{0., 0., -100.},
    };
    linebuf.push_data(ls, 24);
    auto &linevao = r.vaos[r.vaos[GLVao{}]];
    linevao.configure_binding(0, linebufid, 12, 0);
    linevao.configure_attributes({
        eng::GLVaoAttributeDescriptor{0, 0, 3, 0},
    });

    engine.gui_->add_draw([&] {
        if (ImGui::Button("recompile")) { r.programs[rectid].recompile(); }
    });

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);

    while (!window->should_close()) {
        float time = glfwGetTime();
        glfwPollEvents();
        glClearColor(0., 0., 0., 1.);
        Engine::instance().window()->clear_framebuffer();
        eng::Engine::instance().controller()->update();
        cam.update();

        r.programs[rectid].use();
        r.programs[rectid].set("proj", cam.perspective_matrix());
        r.programs[rectid].set("view", cam.view_matrix());
        linevao.bind();
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDrawArraysInstanced(GL_LINES, 0, 2, 100);

        eng::Engine::instance().gui_->draw();
        window->swap_buffers();
    }

    return 0;
}
