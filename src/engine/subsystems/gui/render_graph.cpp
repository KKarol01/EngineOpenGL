#include "render_graph.hpp"

#include <imgui/imgui.h>

#include "../../engine.hpp"

RenderGraphGUI::RenderGraphGUI() {
    buffers_names[0] = "empty";

    add_node(NodeType::VAO);
    add_node(NodeType::VAO);
}

constexpr static auto gvec2(ImVec2 v) { return glm::vec2{v.x, v.y}; }
constexpr static auto ivec2(glm::vec2 v) { return ImVec2{v.x, v.y}; }
constexpr static auto icol32(glm::u8vec4 v) { return IM_COL32(v.r, v.g, v.b, v.a); }

struct VaoBinding {
    uint32_t id, bindingid;
    uint32_t offset, stride;
};
struct VaoAttrib {
    uint32_t id, bindingid;
    uint32_t size, offset;
    eng::GL_FORMAT_ gl_type = eng::GL_FORMAT_FLOAT;
    bool normalize;
};

void RenderGraphGUI::draw() {
    fgdraw_list = ImGui::GetForegroundDrawList();
    // scale       = fmaxf(scale_min, fminf(scale_max, scale + ImGui::GetIO().MouseWheel * .1));

    if (ImGui::Begin("render graph", &open)) {
        ImGui::SetWindowFontScale(1.f);
        mouse_node_interactions();
        can_move_nodes = true;
        if (ImGui::BeginChild("resource_view", ImVec2(200.f, 0.f), true, ImGuiWindowFlags_NoMove)) {
            if (ImGui::TreeNode("Buffers")) {
                ImGui::Unindent(ImGui::GetStyle().IndentSpacing);

                if (ImGui::Button("Add buffer")) {
                    buffers_names[buffers_names.size()] = std::string{"Buffer"} + std::to_string(buffers_names.size());
                }

                for (auto &[id, name] : buffers_names) {
                    if (id == 0) continue;

                    ImGui::PushID(id + 1);

                    if (editing_buffer_name == (int)id) {
                        if (ImGui::InputText("##new_buffer_name",
                                             new_buffer_name,
                                             512,
                                             ImGuiInputTextFlags_CharsNoBlank | ImGuiInputTextFlags_EnterReturnsTrue)) {
                            name = new_buffer_name;
                            delete[] new_buffer_name;
                            editing_buffer_name = -1;
                        }

                    } else {
                        if (ImGui::Selectable(name.c_str(), false, ImGuiSelectableFlags_AllowDoubleClick)) {
                            if (ImGui::IsMouseDoubleClicked(0)) {
                                editing_buffer_name = id;
                                new_buffer_name     = new char[512];
                                memcpy(new_buffer_name, name.c_str(), name.size() + 1);
                            }
                        }
                        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
                            can_move_nodes    = false;
                            resource_dragging = true;
                            ImGui::SetDragDropPayload("vao_binding", &id, 4);
                            ImGui::Text(name.c_str());
                            ImGui::EndDragDropSource();
                        }

                        if (resource_dragging && ImGui::IsMouseDown(0) == false) {
                            resource_dragging = false;
                            can_move_nodes    = true;
                        }
                    }

                    ImGui::PopID();
                }

                ImGui::TreePop();
            }
        }
        ImGui::EndChild();

        ImGui::SameLine();

        canvas_start = gvec2(ImGui::GetCursorScreenPos());
        canvas_size  = gvec2(ImGui::GetContentRegionAvail());
        if (ImGui::BeginChild("canvas", ImVec2(0, 0), true, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar)) {

            ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(63, 63, 63, 255));
            draw_nodes();
            ImGui::PopStyleColor();
        }
        ImGui::EndChild();
    }
    ImGui::End();
}

void RenderGraphGUI::draw_nodes() {

    auto child_to_push_on_top = -1;

    for (auto i = 0u; i < nodes.size(); ++i) {
        auto &node            = nodes.at(i);
        auto nsize            = node.size * scale;
        const auto font_scale = nsize.y / node.size.y;

        if (node.opened == false) { nsize.y = ImGui::GetFrameHeight(); }

        ImGui::SetWindowFontScale(font_scale);
        ImGui::SetCursorPos(ivec2(node.position));

        ImGui::PushStyleColor(ImGuiCol_MenuBarBg, icol32(Colors::menubar.at(node.type)));
        ImGui::PushStyleColor(ImGuiCol_ChildBg, icol32(Colors::node));
        if (ImGui::BeginChild(node.id + 1,
                              ivec2(nsize),
                              true,
                              ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoScrollbar
                                  | ImGuiWindowFlags_NoScrollWithMouse)) {
            draw_node_contents(&node);
        }

        node.hovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem);
        node.down    = node.hovered && ImGui::IsMouseDown(0);
        node.clicked = node.hovered && ImGui::IsMouseClicked(0);
        if (node.clicked) { child_to_push_on_top = (int)i; }

        ImGui::EndChild();
        ImGui::PopStyleColor(2);
    }

    if (child_to_push_on_top != -1) {
        if (inode) {

            if (inode == &nodes.back()) {
                inode = &nodes.at(child_to_push_on_top);
            } else if (inode == &nodes.at(child_to_push_on_top)) {
                inode = &nodes.back();
            }
        }

        auto last                      = nodes.back();
        nodes.back()                   = std::move(nodes.at(child_to_push_on_top));
        nodes.at(child_to_push_on_top) = std::move(last);
    }
}

void RenderGraphGUI::draw_node_contents(Node *node) {
    switch (node->type) {
    case NodeType::DepthTest: {
        break;
    }
    case NodeType::VAO: {
        if (ImGui::BeginMenuBar()) {
            if (ImGui::ArrowButton("vao_fold_toggle", node->opened ? ImGuiDir_Down : ImGuiDir_Right)) {
                node->opened = !node->opened;
            }
            ImGui::SameLine();
            ImGui::Text("VAO");
            ImGui::EndMenuBar();
        }
        auto cpos = ImGui::GetCursorPos();
        cpos.x += ImGui::GetContentRegionAvail().x;
        cpos.x -= ImGui::CalcTextSize("Vao").x;
        auto cd1 = cpos;
        ImGui::SetCursorPos(cpos);
        ImGui::TextColored(ImVec4(255, 0, 0, 255), "Vao");
        cpos       = ImGui::GetCursorPos();
        auto cd2   = cpos.y - cd1.y;
        auto tcpos = cpos;
        tcpos.y -= ImGui::GetFrameHeightWithSpacing() * .5f;
        tcpos.x += node->size.x - 9.f;
        ImGui::SetCursorPos(tcpos);
        node->connection_dots["vao_output"] = gvec2(ImGui::GetCursorScreenPos());

        fgdraw_list->AddCircleFilled(ImGui::GetCursorScreenPos(), 4.f, IM_COL32(0, 255, 0, 255));
        ImGui::SetCursorPos(cpos);

        auto &bindings = *std::any_cast<std::vector<VaoBinding>>(&node->storage.at("bindings"));
        if (ImGui::Button("Add binding", ImVec2(ImGui::GetContentRegionAvail().x, 0))) {
            VaoBinding b;
            b.id        = bindings.size();
            b.bindingid = 0;
            b.offset    = 0;
            b.stride    = 0;

            bindings.push_back(b);
        }

        ImGui::PushStyleColor(ImGuiCol_TableRowBgAlt, icol32(glm::vec4{Colors::node} * 1.55f));
        auto &table_height = *std::any_cast<float>(&node->storage.at("bindings_table_height"));
        if (ImGui::BeginTable("vao_bindings_table",
                              4,
                              ImGuiTableFlags_BordersH | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY,
                              ImVec2(0, table_height))) {
            ImGui::TableSetupColumn("id");
            ImGui::TableSetupColumn("binding");
            ImGui::TableSetupColumn("stride");
            ImGui::TableSetupColumn("offset");
            ImGui::TableSetupScrollFreeze(0, 1);
            ImGui::TableHeadersRow();

            for (auto i = 0u; i < bindings.size(); ++i) {
                ImGui::TableNextRow();

                auto &binding = bindings.at(i);

                ImGui::TableSetColumnIndex(0);
                if (ImGui::Selectable(std::to_string(binding.id).c_str(),
                                      false,
                                      ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap,
                                      ImVec2(0, ImGui::GetFrameHeight()))) {}
                if (ImGui::IsItemActive() && ImGui::IsItemHovered() == false) {
                    int swap_bindings = i;
                    int dir           = ImGui::GetMouseDragDelta(0).y > 0.f ? 1 : -1;

                    if (swap_bindings + dir >= 0 && swap_bindings + dir < bindings.size()) {
                        auto temp                        = bindings.at(swap_bindings);
                        bindings.at(swap_bindings)       = bindings.at(swap_bindings + dir);
                        bindings.at(swap_bindings + dir) = temp;
                        ImGui::ResetMouseDragDelta();
                    }
                }
                ImGui::PushID(i + 1);
                ImGui::TableSetColumnIndex(1);
                ImGui::Text(buffers_names.at(binding.bindingid).c_str());
                if (ImGui::BeginDragDropTarget()) {
                    if (auto payload = ImGui::AcceptDragDropPayload("vao_binding")) {
                        binding.bindingid = *static_cast<uint32_t *>(payload->Data);
                    }
                    ImGui::EndDragDropTarget();
                }

                ImGui::TableSetColumnIndex(2);
                if (int input = binding.stride; ImGui::InputInt("##stride", &input, 0, 0)) {
                    binding.stride = glm::max(0, input);
                }
                ImGui::TableSetColumnIndex(3);
                if (int input = binding.offset; ImGui::InputInt("##offset", &input, 0, 0)) {
                    binding.offset = glm::max(0, input);
                }
                ImGui::PopID();
            }
            ImGui::EndTable();
        }

        ImGui::SetCursorPosY(ImGui::GetCursorPosY() - ImGui::GetStyle().ItemSpacing.y);
        if (ImGui::Button("slider", ImVec2(FLT_MAX, 10.f))) {}
        if (ImGui::IsItemActive()) {
            can_move_nodes = false;
            table_height += ImGui::GetMouseDragDelta(0, 0.f).y;
            ImGui::ResetMouseDragDelta();
        }
        auto &attributes = *std::any_cast<std::vector<VaoAttrib>>(&node->storage.at("attribs"));
        if (ImGui::Button("Add attribute", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetFrameHeight()))) {
            VaoAttrib a;
            a.id        = attributes.size();
            a.bindingid = 0;
            a.size      = 0;
            a.offset    = 0;
            a.gl_type   = eng::GL_FORMAT_FLOAT;
            a.normalize = false;
            attributes.push_back(a);
        }

        if (ImGui::BeginTable("vao_attributes_table",
                              6,
                              ImGuiTableFlags_BordersH | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY)) {
            ImGui::TableSetupColumn("id", ImGuiTableColumnFlags_WidthStretch, 40.f);
            ImGui::TableSetupColumn("binding", ImGuiTableColumnFlags_WidthStretch, 40.f);
            ImGui::TableSetupColumn("size", ImGuiTableColumnFlags_WidthStretch, 40.f);
            ImGui::TableSetupColumn("offset", ImGuiTableColumnFlags_WidthStretch, 40.f);
            ImGui::TableSetupColumn("gl_type", ImGuiTableColumnFlags_WidthStretch, 75.f);
            ImGui::TableSetupColumn("N", ImGuiTableColumnFlags_WidthStretch, 20.f);
            ImGui::TableSetupScrollFreeze(0, 1);
            ImGui::TableHeadersRow();

            for (auto i = 0u; i < attributes.size(); ++i) {
                auto &attrib = attributes.at(i);
                ImGui::TableNextRow();

                ImGui::PushID(i + 1);

                ImGui::TableSetColumnIndex(0);
                if (int input = attrib.id; ImGui::InputInt("##id", &input, 0, 0)) { attrib.id = fmaxf(0, input); }
                ImGui::TableSetColumnIndex(1);
                if (int input = attrib.bindingid; ImGui::InputInt("##binding", &input, 0, 0)) {
                    attrib.bindingid = fmaxf(0, input);
                }
                ImGui::TableSetColumnIndex(2);
                if (int input = attrib.size; ImGui::InputInt("##size", &input, 0, 0)) { attrib.size = fmaxf(0, input); }
                ImGui::TableSetColumnIndex(3);
                if (int input = attrib.offset; ImGui::InputInt("##offset", &input, 0, 0)) {
                    attrib.offset = fmaxf(0, input);
                }
                ImGui::TableSetColumnIndex(4);
                const char *types[]{"GL_FLOAT"};
                if (ImGui::BeginCombo("##gl_type", types[(int)attrib.gl_type])) {
                    for (int i = 0; i < IM_ARRAYSIZE(types); ++i) {
                        if (ImGui::Selectable(types[i])) { attrib.gl_type = eng::GL_FORMAT_(i); }
                    }

                    ImGui::EndCombo();
                }
                ImGui::TableSetColumnIndex(5);
                ImGui::Checkbox("##normalize", &attrib.normalize);
                ImGui::PopID();
            }

            ImGui::EndTable();
        }
        ImGui::PopStyleColor();
        break;
    }
    }
}

void RenderGraphGUI::add_node(NodeType type) {
    Node n;
    n.type     = type;
    n.position = {0.f, 0.f};

    switch (type) {
    case NodeType::DepthTest: break;
    case NodeType::VAO:
        n.size                             = glm::vec2{400.f, 500.f};
        n.storage["bindings"]              = std::vector<VaoBinding>{};
        n.storage["attribs"]               = std::vector<VaoAttrib>{};
        n.storage["bindings_table_height"] = n.size.y * .45f;
        break;

    default: break;
    }

    n.min_size = .05f * n.size;
    n.max_size = 1.5f * n.size;
    nodes.push_back(std::move(n));
}

Node *RenderGraphGUI::find_node(std::function<bool(Node *)> pred) {
    for (auto it = nodes.rbegin(); it != nodes.rend(); ++it) {
        if (pred(&*it)) return &*it;
    }

    return nullptr;
}

void RenderGraphGUI::mouse_node_interactions() {
    if (can_move_nodes == false) return;

    const auto mousepos    = gvec2(ImGui::GetMousePos());
    const auto mouserelpos = mousepos - canvas_start;

    if (inode) {
        if (draw_line) {
            if (ImGui::IsMouseReleased(0)) {
                draw_line = false;
                inode     = nullptr;
                return;
            }

            const auto a = line_start;
            const auto b = mousepos;
            fgdraw_list->AddLine(ivec2(a), ivec2(b), IM_COL32(255, 255, 0, 255), 4.f);

        } else if (inode->resizing) {
            ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);

            if (ImGui::IsMouseDown(0) == false) {
                inode->resizing = false;
                inode           = nullptr;
                return;
            }

            inode->size.x += ImGui::GetMouseDragDelta(0, 0.f).x;
            ImGui::ResetMouseDragDelta();
        } else if (inode->moving) {
            if (ImGui::IsMouseDown(0) == false) {
                inode->moving = false;
                inode         = nullptr;
                return;
            }

            inode->position += gvec2(ImGui::GetMouseDragDelta(0, 0.f));
            ImGui::ResetMouseDragDelta();
        }

        return;
    }

    const auto require_visibility = [&]() {
        if (!inode) return;

        const auto visible = inode && inode == find_node([](auto node) { return node->hovered; });
        if (!visible) { inode = nullptr; }
    };

    inode = find_node([&](Node *n) {
        if (ImGui::IsMouseDown(0) == false) return false;

        for (const auto &[name, v] : n->connection_dots) {
            if (glm::distance(mousepos, v) <= 8.f) {
                draw_line  = true;
                line_start = v;
                return true;
            }
        }

        return false;
    });
    if (inode && draw_line) { return; }

    inode = find_node([&](Node *n) {
        const auto p   = mouserelpos;
        const auto np  = n->position;
        const auto ns  = n->size * scale;
        const auto nps = np + ns;
        const auto r   = p - glm::vec2{nps.x, np.y};
        return fabsf(r.x) < 5.f && r.y >= 0.f && r.y <= ns.y;
    });
    require_visibility();

    if (inode) {
        ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
        if (ImGui::IsMouseClicked(0)) {
            inode->resizing = true;
        } else {
            inode = nullptr;
        }
        return;
    }
    ImGui::SetMouseCursor(ImGuiMouseCursor_Arrow);

    inode = find_node([&](Node *n) { return n->clicked; });
    require_visibility();
    if (inode) { inode->moving = true; }
}
