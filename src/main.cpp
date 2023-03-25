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
#include <imgui/imgui_stdlib.h>
#include <glm/gtx/euler_angles.hpp>

#include "engine/subsystems/gui/gui.hpp"
#include "engine/model/importers.hpp"
#include "engine/renderer/renderer.hpp"
#include "engine/wrappers/buffer/ubo.hpp"
#include "engine/allocators/idallocator.hpp"
#include "engine/physics/gjk.hpp"

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
    auto cam          = Camera{};
    using namespace eng;

    // auto rect_program    = engine.renderer_->programs.emplace("rect");
    /*auto gjk_test_cube  = imp.import_model("3dmodels/simple_shapes/cube.obj", aiProcess_Triangulate);
    auto gjk_test_plane = imp.import_model("3dmodels/simple_shapes/plane.obj", aiProcess_Triangulate);*/
    auto default_program = engine.renderer_->create_program("cube");
    auto vao             = engine.renderer_->create_vao();
    auto vbo             = engine.renderer_->create_buffer();
    auto ebo             = engine.renderer_->create_buffer();

    {
        vao->configure_binding(0, vbo, 8, 0);
        vao->configure_ebo(ebo);
        vao->configure_attribute(eng::GL_ATTR_0, 0, 2, 0);

        uint32_t indices[4]{0, 2, 1, 3};
        glm::vec2 vertices[4]{
            {0.f, 0.f},
            {1.f, 0.f},
            {0.f, 1.f},
            {1.f, 1.f},
        };

        vbo->push_data(vertices, sizeof(vertices));
        ebo->push_data(indices, sizeof(indices));
    }

    auto &renderer      = *engine.renderer_;
    auto pp             = renderer.create_pipeline();
    auto &pps1          = pp->create_stage();
    pps1.program        = default_program;
    pps1.vao            = vao;
    pps1.draw_cmd       = std::make_shared<eng::DrawElementsInstancedCMD>(vbo, 1000000);
    pps1.on_stage_start = [&]() {
        default_program->set("model", glm::mat4{1.f});
        default_program->set("view", cam.view_matrix());
        default_program->set("projection", cam.perspective_matrix());
    };

    uint32_t fbo, rbo, coltex, depthtex;
    glCreateFramebuffers(1, &fbo);
    glCreateTextures(GL_TEXTURE_2D, 1, &coltex);
    glCreateTextures(GL_TEXTURE_2D, 1, &depthtex);
    glTextureParameteri(coltex, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(coltex, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(coltex, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(coltex, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTextureStorage2D(coltex, 1, GL_RGBA8, window->width() * .75f, window->height() * .75f);
    glTextureParameteri(depthtex, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(depthtex, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(depthtex, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(depthtex, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTextureStorage2D(depthtex, 1, GL_DEPTH24_STENCIL8, window->width() * .75f, window->height() * .75f);

    glNamedFramebufferTexture(fbo, GL_COLOR_ATTACHMENT0, coltex, 0);
    glNamedFramebufferTexture(fbo, GL_DEPTH_STENCIL_ATTACHMENT, depthtex, 0);

    assert(glCheckNamedFramebufferStatus(fbo, GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
    std::vector<std::string> lines{10};
    engine.gui_->add_draw([&]() {
        auto window_w = static_cast<float>(window->width());
        auto window_h = static_cast<float>(window->height());

        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2((float)window->width(), (float)window->height()));
        if (ImGui::Begin("##main_window", 0, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDecoration)) {
            if (ImGui::BeginMenuBar()) {
                if (ImGui::Button("Open project")) {}
                if (ImGui::Button("Save project")) {}
                if (ImGui::Button("Save project as")) {}
                ImGui::EndMenuBar();
            }

            if (ImGui::BeginChild("##render", ImVec2(window_w * .75f, window_h * .75f), true)) {
                ImGui::Image((void *)coltex, ImGui::GetContentRegionAvail(), ImVec2(0, 1), ImVec2(1, 0));
            }
            ImGui::EndChild();
            auto cpos = ImGui::GetCursorPos();

            ImGui::SameLine();

            if (ImGui::BeginChild("##equation", ImVec2(0, 0), true)) {
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                if (ImGui::BeginCombo("##Add...", "Add...", ImGuiComboFlags_NoArrowButton)) {
                    if (ImGui::Selectable("Function")) { printf("aa"); }
                    if (ImGui::Selectable("Constant")) { printf("bb"); }
                    if (ImGui::Selectable("Slider")) { printf("cc"); }
                    ImGui::EndCombo();
                }

                ImGui::PushStyleColor(ImGuiCol_FrameBg, ImGui::GetStyleColorVec4(ImGuiCol_ChildBg));

                if (ImGui::BeginListBox("##equations", ImVec2(ImGui::GetContentRegionAvail()))) {
                    ImGui::PopStyleColor();
                    for (int i = 0; i < 10; ++i) {
                        ImGui::PushID(i + 1);
                        ImGui::BeginGroup();
                        auto c = ImGui::GetCursorPos();
                        ImGui::Selectable("##sel",
                                          false,
                                          ImGuiSelectableFlags_AllowItemOverlap,
                                          ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetFrameHeight()));
                        if (ImGui::IsItemActive() && ImGui::IsItemHovered() == false) {
                            auto dir = ImGui::GetMouseDragDelta().y < 0.f ? -1 : 1;
                            printf("%i", dir);
                            //ImGui::ResetMouseDragDelta();
                        }

                        ImGui::SetCursorPos(c);
                        ImGui::AlignTextToFramePadding();
                        ImGui::Text("f(x,z) = ");
                        ImGui::SameLine();
                        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                        ImGui::InputText("##eq", &lines[i]);
                        ImGui::EndGroup();

                        ImGui::PopID();
                    }
                    ImGui::EndListBox();
                }
            }
            ImGui::EndChild();

            ImGui::SetCursorPos(cpos);
            if (ImGui::BeginChild("Errors", ImVec2(window_w * .75f, 0), true)) { ImGui::Text("a"); }
            ImGui::EndChild();
        }
        ImGui::End();
    });

    Engine::instance().window()->set_clear_flags(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glClearColor(.25f, .25f, .25f, 0.f);
    while (!window->should_close()) {
        float time = glfwGetTime();
        glfwPollEvents();

        eng::Engine::instance().controller()->update();
        cam.update();

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glViewport(0, 0, (float)(window->width()) * .75f, (float)(window->height()) * .75f);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        window->clear_framebuffer();
        engine.renderer_->render_frame();

        window->adjust_glviewport();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        window->clear_framebuffer();
        eng::Engine::instance().gui_->draw();
        window->swap_buffers();
    }

    return 0;
}
