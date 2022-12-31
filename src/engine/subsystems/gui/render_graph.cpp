#include "render_graph.hpp"

#include <imgui/imgui.h>

#include "../../engine.hpp"

RenderGraphGUI::RenderGraphGUI() { add_node(NodeType::VAO); }

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
        auto &node = nodes.at(i);

        static bool opened = true;
        const auto mw      = ImGui::GetIO().MouseWheel * .1f;
        const auto aspect  = node.size.x / node.size.y;
        node.size.x += zoom_speed * mw * aspect;
        node.size.y += zoom_speed * mw;

        const auto cspos = gvec2(ImGui::GetCursorScreenPos());
        if (ImGui::BeginChild("test", ImVec2(0, 0), true, ImGuiWindowFlags_NoMove)) {
            const auto cursor     = gvec2(ImGui::GetCursorPos());
            const auto font_scale = node.size.y / nodes_original_sizes.at(node.id).y;

            ImGui::SetWindowFontScale(font_scale);
            ImGui::SetCursorPos(ivec2(node.position));

            const auto child_size = ImVec2(node.size.x, opened ? node.size.y : ImGui::GetFrameHeight());
            if (ImGui::BeginChild(1, child_size, true, ImGuiWindowFlags_MenuBar)) {
                if (ImGui::BeginMenuBar()) {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ChildBg));
                    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImGui::GetStyleColorVec4(ImGuiCol_ChildBg));
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGui::GetStyleColorVec4(ImGuiCol_ChildBg));
                    if (ImGui::ArrowButton("close_btn", opened ? ImGuiDir_Down : ImGuiDir_Right)) { opened = !opened; }
                    ImGui::PopStyleColor(3);
                    ImGui::SameLine();
                    ImGui::TextWrapped("VAO");

                    ImGui::EndMenuBar();
                }

                ImGui::Text("some text");
            }
            ImGui::EndChild();

            static bool clicked  = false;
            static bool resizing = false;
            auto mp              = gvec2(ImGui::GetIO().MousePos);
            mp -= cspos;

            if (ImGui::IsItemClicked(0)) {
                printf("asdf");
                clicked = true; }
            if (fabs(mp.x - node.position.x - node.size.x) < 10.f && mp.y - node.position.y >= 0.f
                && mp.y - node.position.y <= node.size.y) {
                if (ImGui::IsMouseClicked(0)) resizing = true;
                ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
            } else {
                ImGui::SetMouseCursor(ImGuiMouseCursor_Arrow);
            }

            if (resizing) {
                static bool dragging = false;
                dragging             = dragging || ImGui::IsMouseDown(0);

                if (dragging) {
                    node.size.x += ImGui::GetMouseDragDelta(0,0.f).x;
                    ImGui::ResetMouseDragDelta();
                } else {
                    resizing = false;
                }

                if (dragging && ImGui::IsMouseReleased(0) == true) {
                    resizing = false;
                    dragging = false;
                }

            } else if (clicked) {
                node.position += gvec2(ImGui::GetMouseDragDelta(0, 0.f));
                ImGui::ResetMouseDragDelta();

                if (ImGui::IsMouseDown(0) == false) { clicked = false; }
            }
        }
        ImGui::EndChild();
    }
}

void RenderGraphGUI::add_node(NodeType type) {
    Node n;
    n.type     = type;
    n.position = {0.f, 0.f};
    n.size     = glm::vec2{150.f, 100.f};
    n.min_size = .25f * n.size;
    n.max_size = 1.25f * n.size;

    nodes_original_sizes[n.id] = n.size;
    nodes.push_back(std::move(n));
}