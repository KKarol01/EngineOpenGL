#include "render_graph.hpp"

#include <imgui/imgui.h>

#include "../../engine.hpp"

RenderGraphGUI::RenderGraphGUI() {}

constexpr static auto gvec2(ImVec2 v) { return glm::vec2{v.x, v.y}; }
constexpr static auto ivec2(glm::vec2 v) { return ImVec2{v.x, v.y}; }

void RenderGraphGUI::draw() {

    if (ImGui::Begin("render graph", &open)) {
        ImGui::SetWindowFontScale(1.f);

        draw_nodes();
    }
    ImGui::End();
}

void RenderGraphGUI::draw_nodes() {
    for (auto i = 0u; i < nodes.size(); ++i) {
        static bool opened = true;
        const auto mw      = ImGui::GetIO().MouseWheel * .02f;
        w += 5.f * mw * 50.f * aspect;
        h += 5.f * mw * 50.f;

        float sx = ImGui::GetCursorScreenPos().x;
        float sy = ImGui::GetCursorScreenPos().y;
        if (ImGui::BeginChild("test", ImVec2(0, 0), true, ImGuiWindowFlags_NoMove)) {

            ImGui::SetWindowFontScale(h / oh);

            static float offx = 0.f;
            static float offy = 0.f;

            ImGui::SetCursorPosX(offx);
            ImGui::SetCursorPosY(offy);

            const auto child_size = ImVec2(w, opened ? h : ImGui::GetFrameHeight());
            if (ImGui::BeginChild(1, child_size, true, ImGuiWindowFlags_MenuBar)) {
                if (ImGui::BeginMenuBar()) {

                    if (ImGui::ArrowButton("close_btn", opened ? ImGuiDir_Down : ImGuiDir_Right)) { opened = !opened; }
                    ImGui::SameLine();
                    ImGui::TextWrapped("VAO");

                    ImGui::EndMenuBar();
                }

                ImGui::Text("some text");
            }
            ImGui::EndChild();

            static bool clicked  = false;
            static bool resizing = false;
            auto mp              = ImGui::GetIO().MousePos;

            if (ImGui::IsItemClicked(0)) { clicked = true; }
            if (fabs(mp.x - sx - w - offx) < 10.f && mp.y - sy - offy >= 0.f && mp.y - sy - offy <= h) {
                resizing = true;
                ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
            } else {
                ImGui::SetMouseCursor(ImGuiMouseCursor_Arrow);
            }

            if (resizing) {
                static bool dragging = false;
                dragging             = dragging || ImGui::IsMouseDown(0);

                if (dragging) {
                    w += ImGui::GetMouseDragDelta(0, 0.f).x;
                    ImGui::ResetMouseDragDelta();
                } else {
                    resizing = false;
                }

                if (dragging && ImGui::IsMouseReleased(0) == true) {
                    resizing = false;
                    dragging = false;
                }

            } else if (clicked && ImGui::IsMouseDragging(0)) {
                offx += ImGui::GetMouseDragDelta().x;
                offy += ImGui::GetMouseDragDelta().y;
                ImGui::ResetMouseDragDelta();

                if (ImGui::IsMouseDown(0) == false) { clicked = false; }
            }
        }
        ImGui::EndChild();
    }
}
