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

    auto default_program = engine.renderer_->programs.emplace("cube");
    auto rect_program    = engine.renderer_->programs.emplace("rect");

    ModelImporter imp;
    auto gjk_test_cube  = imp.import_model("3dmodels/simple_shapes/cube.obj", aiProcess_Triangulate);
    auto gjk_test_plane = imp.import_model("3dmodels/simple_shapes/plane.obj", aiProcess_Triangulate);

    auto pp2 = engine.renderer_->pipelines.emplace();
    PipelineStage pp2s1, pp2s2;
    auto pp2s1vao = engine.renderer_->vaos.emplace();
    pp2s1vao->configure_binding(0, pp2->vbo, 12, 0);
    pp2s1vao->configure_attributes(GL_FLOAT, 4, {GLVaoAttributeSameType{0, 0, 3}});
    {
        float line_verts[]{0.f, 0.f, -50.f, 0.f, 0.f, 50.f};
        engine.renderer_->buffers[pp2->vbo].push_data(line_verts, sizeof(line_verts));
    }
    pp2s1.vao            = pp2s1vao;
    pp2s1.program        = rect_program.id;
    pp2s1.draw_cmd       = std::make_shared<DrawArraysInstancedBaseInstanceCMD>(2, 101, 0, 0, GL_LINES);
    pp2s2.draw_cmd       = std::make_shared<DrawArraysInstancedBaseInstanceCMD>(2, 101, 0, 102, GL_LINES);
    pp2s1.on_stage_start = [&] {
        rect_program->set("model", glm::mat4{1.f});
        rect_program->set("view", cam.view_matrix());
        rect_program->set("projection", cam.perspective_matrix());
    };
    pp2s2.on_stage_start = [&] {
        static const auto R = glm::rotate(glm::mat4{1.f}, 3.14f / 2.f, glm::vec3{0, 1, 0});
        rect_program->set("model", R);
    };
    pp2->stages.push_back(std::move(pp2s1));
    pp2->stages.push_back(std::move(pp2s2));

    eng::Engine::instance().gui_->add_draw([&] {
        static auto r = 5.f;
        static auto w = 160.f, h = 40.f;

        // ImGuiStyleVar_FramePadding

        ImGui::Begin("render graph");

        ImGui::BeginChild("node settings",
                          ImVec2(0, ImGui::GetTextLineHeightWithSpacing() * 4.f),
                          true,
                          ImGuiWindowFlags_NoMove );
        ImGui::SetNextItemWidth(100.f);
        ImGui::BeginGroup();
        ImGui::DragFloat("window rounding radius", &r, 1.f, 0.f, 100.f);
        ImGui::SetNextItemWidth(100.f);
        ImGui::DragFloat("window w", &w, 2.f, 0.f, 1000.f);
        ImGui::EndGroup();
        ImGui::SameLine();
        ImGui::SetNextItemWidth(100.f);
        ImGui::DragFloat("window h", &h, 2.f, 0.f, 1000.f);
        ImGui::EndChild();

        ImGui::BeginChild("render-graph-buttons", ImVec2(0.f, ImGui::GetTextLineHeightWithSpacing() * 2.f), true, ImGuiWindowFlags_NoScrollbar);
        ImGui::SameLine(); ImGui::Button("Program");
        ImGui::SameLine(); ImGui::Button("Texture");
        ImGui::SameLine(); ImGui::Button("Stage");
        ImGui::SameLine(); ImGui::Button("DrawCmd");
        ImGui::EndChild();

        ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(55, 55, 55, 200)); // Set a background color
        ImGui::BeginChild("canvas", ImVec2(0.0f, 0.0f), true, ImGuiWindowFlags_NoMove);

        auto &io                   = ImGui::GetIO();
        auto draw_list             = ImGui::GetWindowDrawList();
        auto canvas_size           = ImGui::GetContentRegionAvail();
        auto canvas_min            = ImGui::GetCursorScreenPos();
        auto canvas_max            = ImVec2(canvas_min.x + canvas_size.x, canvas_min.y + canvas_size.y);
        auto mouse_pos             = io.MousePos;
        static auto gscroll_offset = ImVec2(0.f, 0.f);

        auto grid_step = 64.f;

        for (float i = fmodf(gscroll_offset.x, grid_step); i < canvas_size.x; i += grid_step) {
            draw_list->AddLine(ImVec2(canvas_min.x + i, canvas_min.y),
                               ImVec2(canvas_min.x + i, canvas_max.y),
                               IM_COL32(200, 200, 200, 40));
        }
        for (float i = fmodf(gscroll_offset.y, grid_step); i < canvas_size.y; i += grid_step) {
            draw_list->AddLine(ImVec2(canvas_min.x, canvas_min.y + i),
                               ImVec2(canvas_max.x, canvas_min.y + i),
                               IM_COL32(200, 200, 200, 40));
        }

        static std::vector<ImVec2> rects{{{20.f, 20.f}, {100.f, 100.f}}};
        static constexpr auto corner
            = [](float dx, float dy, float r) { return dx > 0.f || dy > 0.f || dx * dx + dy * dy < r * r; };
        static constexpr auto is_rect_hovered = [](float mx, float my) {
            return mx > 0.f && my > 0.f && mx < w && my < h && corner(mx - r, my - r, r)
                   && corner(w - mx - r, my - r, r) && corner(mx - r, h - my - r, r)
                   && corner(w - mx - r, h - my - r, r);
        };

        if (
            ImGui::IsWindowFocused()&&
            ImGui::IsMouseDragging(1)) {
            gscroll_offset.x += io.MouseDelta.x;
            gscroll_offset.y += io.MouseDelta.y;
        }

        static auto selected = -1;
        for (auto i = 0; i < rects.size(); ++i) {
            auto rposmin = ImVec2(canvas_min.x + rects.at(i).x + gscroll_offset.x,
                                  canvas_min.y + rects.at(i).y + gscroll_offset.y);
            auto rposmax = ImVec2(rposmin.x + w, rposmin.y + h);

            auto rect_bg = IM_COL32(155, 155, 155, 50);
            auto relmp   = mouse_pos;
            relmp.x -= rposmin.x, relmp.y -= rposmin.y;

            if (ImGui::IsWindowHovered()) {
                if (selected == -1 && is_rect_hovered(relmp.x, relmp.y)) {
                    selected = i;
                } else {
                    selected = -1;
                }

                if (selected == i) {
                    rect_bg = IM_COL32(155, 155, 155, 150);

                    if (ImGui::IsMouseDown(0)) rect_bg |= 0xFF000000;
                    if (ImGui::IsMouseDragging(0, .01f)) {
                        rects.at(i).x += io.MouseDelta.x;
                        rects.at(i).y += io.MouseDelta.y;
                    }
                }
            }

            draw_list->AddRectFilled(rposmin, rposmax, rect_bg, r);
        }

        static std::vector<std::pair<int, int>> rectconns{{{0, 1}}};

        for (const auto &[a, b] : rectconns) {
            auto rposmina = ImVec2(canvas_min.x + rects.at(a).x + gscroll_offset.x,
                                   canvas_min.y + rects.at(a).y + gscroll_offset.y);
            auto rposminb = ImVec2(canvas_min.x + rects.at(b).x + gscroll_offset.x,
                                   canvas_min.y + rects.at(b).y + gscroll_offset.y);
            auto rposmaxa = ImVec2(rposmina.x + w, rposmina.y + h);
            auto rposmaxb = ImVec2(rposminb.x + w, rposminb.y + h);

            ImVec2 c1{rposmaxa.x, rposmina.y + h * .5f};
            ImVec2 c2{rposminb.x, rposminb.y + h * .5f};
            ImVec2 cp1{c1.x + 80.f, c1.y}, cp2{c2.x - 80.f, c2.y};

            draw_list->AddBezierCubic(c1, cp1, cp2, c2, IM_COL32(55, 191, 114, 255), 3.f);

            auto text_size = ImGui::CalcTextSize("SAMPLE");
            draw_list->AddText(
                ImVec2(((rposmina.x + w * .5f) - text_size.x * .5f), (rposmina.y + h * .5f - text_size.y * .5f)),
                IM_COL32(255, 0, 255, 255),
                "SAMPLE");
        }

        ImGui::EndChild();
        ImGui::PopStyleColor();
        ImGui::End();
    });

    while (!window->should_close()) {
        float time = glfwGetTime();
        glfwPollEvents();
        glClearColor(.25f, .25f, .25f, 1.);
        Engine::instance().window()->set_clear_flags(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        Engine::instance().window()->clear_framebuffer();
        eng::Engine::instance().controller()->update();
        cam.update();

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_DEPTH_TEST);
        engine.renderer_->render();

        eng::Engine::instance().gui_->draw();
        window->swap_buffers();
    }

    return 0;
}
