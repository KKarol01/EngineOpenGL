#include "render_graph.hpp"

#include <imgui/imgui.h>

#include "../../engine.hpp"

typedef std::vector<std::pair<int, int>> VaoBindings;

static glm::vec2 convert_vec(ImVec2 v) { return {v.x, v.y}; }
static ImVec2 convert_vec(glm::vec2 v) { return {v.x, v.y}; }
static unsigned convert_vec(glm::u8vec4 v) { return IM_COL32(v.r, v.g, v.b, v.a); }

RenderGraphGUI::RenderGraphGUI() { nodes.emplace_back(node_builder.build(NodeType::VAO)); }

void RenderGraphGUI::draw() {
    if (ImGui::Begin("render graph", &open)) {

        if (ImGui::BeginChild("render-graph-buttons",
                              ImVec2(0.f, ImGui::GetTextLineHeightWithSpacing() * 2.f),
                              true,
                              ImGuiWindowFlags_NoScrollbar)) {
            ImGui::SameLine();
            if (ImGui::Button("Program")) {
                nodes.emplace_back(node_builder.build(NodeType::DepthTest));
                nodes.back().position = -pan_offset + (canvas_size - nodes.back().size) * .5f;
            }
            ImGui::SameLine();
            ImGui::Button("Texture");
            ImGui::SameLine();
            ImGui::Button("Stage");
            ImGui::SameLine();
            ImGui::Button("DrawCmd");
            ImGui::SameLine();
            if (ImGui::Button("Vao")) {
                nodes.emplace_back(node_builder.build(NodeType::VAO));
                nodes.back().position = -pan_offset + (canvas_size - nodes.back().size) * .5f;
            }
        }
        ImGui::EndChild();

        can_move_nodes = true;

        draw_resource_list();
        ImGui::SameLine();
        draw_canvas();
    }
    ImGui::End();
}

void RenderGraphGUI::handle_panning() {
    if (canvas_panning == false && ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows)) { canvas_panning = true; }

    if (canvas_panning) {

        auto &io = ImGui::GetIO();
        if (ImGui::IsMouseDragging(1, 0.5f) == false) {
            canvas_panning = false;
            return;
        }
        pan_offset += convert_vec(io.MouseDelta);
    }
}

void RenderGraphGUI::handle_node_mouse_interaction() {
    if (can_move_nodes == false) return;

    Node *tn{nullptr};
    uint32_t tnidx{0};

    for (auto i = 0u; i < nodes.size(); ++i) {
        auto &n = nodes.at(i);

        if (n.mouse_dragging) {
            tn    = &n;
            tnidx = i;
            break;
        }

        if (mouse_over_node(n)) {
            tn    = &n;
            tnidx = i;
        }

        n.mouse_hovering = false;
        n.mouse_down     = false;
    }

    inode = tn;
    if (inode == nullptr) return;

    inode->mouse_hovering = true;
    inode->mouse_down     = ImGui::IsMouseDown(0);

    // if (inode->mouse_down && nodes.size() > 1 && inode != &nodes.back()) {
    //     Node temp       = std::move(nodes.back());
    //     nodes.back()    = std::move(nodes.at(tnidx));
    //     nodes.at(tnidx) = std::move(temp);
    // }

    if (ImGui::IsMouseDragging(0)) {
        if (inode->mouse_dragging == false) {
            inode->drag_start = inode->drag_end = convert_vec(ImGui::GetIO().MousePos);
            inode->mouse_dragging               = true;
        } else {
            inode->drag_end = convert_vec(ImGui::GetIO().MousePos);
            inode->position += inode->drag_end - inode->drag_start;
            inode->drag_start = inode->drag_end;
        }
    } else
        inode->mouse_dragging = false;
}

void RenderGraphGUI::draw_grid() {
    for (float i = fmodf(pan_offset.x, grid_step); i < canvas_size.x; i += grid_step) {
        draw_list->AddLine(ImVec2(canvas_start.x + i, canvas_start.y),
                           ImVec2(canvas_start.x + i, canvas_end.y),
                           convert_vec(Colors::grid));
    }
    for (float i = fmodf(pan_offset.y, grid_step); i < canvas_size.y; i += grid_step) {
        draw_list->AddLine(ImVec2(canvas_start.x, canvas_start.y + i),
                           ImVec2(canvas_end.x, canvas_start.y + i),
                           convert_vec(Colors::grid));
    }
}

void RenderGraphGUI::draw_nodes() {

    for (auto i = 0u; i < nodes.size(); ++i) {
        auto &n = nodes.at(i);
        auto a  = canvas_start + pan_offset + n.start();
        auto b  = canvas_start + pan_offset + n.end();

        ImGui::SetCursorPos(convert_vec(pan_offset + n.position));
        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, Node::corner_rounding);
        ImGui::PushStyleColor(ImGuiCol_MenuBarBg, IM_COL32(252, 80, 68, 255));

        if (n.mouse_hovering) ImGui::PushStyleColor(ImGuiCol_Border, convert_vec(Colors::node_hovered));
        if (n.mouse_down) ImGui::PushStyleColor(ImGuiCol_Border, convert_vec(Colors::node_clicked));
        if (ImGui::BeginChild(i + 1, ImVec2(n.size.x, n.size.y), true, ImGuiWindowFlags_MenuBar)) {
            if (n.mouse_hovering) ImGui::PopStyleColor();
            if (n.mouse_down) ImGui::PopStyleColor();
            ImGui::PopStyleColor();
            ImGui::PopStyleVar();

            draw_node_contents(&n);

        } else {
            if (n.mouse_hovering) ImGui::PopStyleColor();
            if (n.mouse_down) ImGui::PopStyleColor();
            ImGui::PopStyleVar();
            ImGui::PopStyleColor();
        }
        ImGui::EndChild();

    }
}

void RenderGraphGUI::draw_node_contents(Node *node) {

    static const char *depth_tests[]{"GL_LESS", "GL_GREATER", "GL_NOTEQUAL"};

    switch (node->type) {
    case NodeType::DepthTest: {
        ImGui::BeginMenuBar();
        ImGui::Text("Depth test");
        ImGui::EndMenuBar();

        /*auto &depth_test = *node->storage.get<uint32_t>("depth_test");
        ImGui::PushItemWidth(ImGui::CalcTextSize("GL_NOTEQUAL").x + 25.f);
        if (ImGui::BeginCombo("Depth func", depth_tests[depth_test])) {

            for (auto i = 0u; i < IM_ARRAYSIZE(depth_tests); ++i) {
                if (ImGui::Selectable(depth_tests[i], i == depth_test)) { depth_test = i; }
            }
            ImGui::EndCombo();
        }
        draw_connection_dot(node, false, false);
        ImGui::PopItemWidth();*/
        return;
    }
    case NodeType::VAO: {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 5));
        ImGui::PushStyleColor(ImGuiCol_TableRowBgAlt, convert_vec(Colors::grid));

        ImGui::BeginMenuBar();
        ImGui::Text("VAO");
        ImGui::EndMenuBar();

        static float height = 150;

        auto bindings_ptr = std::any_cast<VaoBindings>(&node->storage.storage["vao_bindings"]);
        assert(bindings_ptr);

        auto &bindings = *bindings_ptr;

        if (ImGui::BeginChild("##bindings", ImVec2(0, height), true)) {

            if (ImGui::BeginTable("buffer_bindings",
                                  3,
                                  ImGuiTableFlags_ScrollY | ImGuiTableFlags_BordersH | ImGuiTableFlags_RowBg,
                                  ImVec2(0, ImGui::GetContentRegionAvail().y))) {
                ImGui::TableSetupColumn("Buffer");
                ImGui::TableSetupColumn("Binding id");
                ImGui::TableSetupColumn("");
                ImGui::TableSetupScrollFreeze(0, 1);

                ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
                ImGui::TableSetColumnIndex(0);
                ImGui::PushID(1);
                ImGui::AlignTextToFramePadding();
                ImGui::TableHeader(ImGui::TableGetColumnName(0));
                ImGui::TableSetColumnIndex(1);
                ImGui::PushID(2);
                ImGui::TableHeader(ImGui::TableGetColumnName(1));
                ImGui::TableSetColumnIndex(2);
                ImGui::PushID(3);
                if (ImGui::Button("+", ImVec2(ImGui::GetContentRegionAvail().x, 0))) {
                    bindings.push_back({bindings.size(), 0});
                }
                ImGui::PopID();
                ImGui::PopID();
                ImGui::PopID();

                for (auto i = 0u; i < bindings.size(); ++i) {

                    int buff    = bindings.at(i).first;
                    int binding = bindings.at(i).second;

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);

                    bool s = false;
                    ImGui::Selectable(std::to_string(buff).c_str(),
                                      &s,
                                      ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap,
                                      ImVec2(0, ImGui::GetTextLineHeight() + ImGui::GetStyle().ItemSpacing.y * 1.5f));

                    if (ImGui::BeginDragDropTarget()) {
                        if (auto payload = ImGui::AcceptDragDropPayload("DND")) {
                            auto data = *static_cast<int *>(payload->Data);
                        }

                        ImGui::EndDragDropTarget();
                    }

                    if (ImGui::IsItemActive()) can_move_nodes = false;
                    if (ImGui::IsItemActive() && ImGui::IsItemHovered() == false) {
                        uint32_t dst_idx = i + (ImGui::GetMouseDragDelta().y < 0.f ? -1 : 1);
                        ImGui::ResetMouseDragDelta(0);
                        if (dst_idx >= 0u && dst_idx < bindings.size()) {
                            auto temp            = bindings.at(dst_idx);
                            bindings.at(dst_idx) = bindings.at(i);
                            bindings.at(i)       = temp;
                        }
                    }

                    ImGui::TableSetColumnIndex(1);
                    if (ImGui::InputInt("##binding", &binding, 0, 1)) { bindings.at(i).second = glm::max(0, binding); }

                    ImGui::TableSetColumnIndex(2);
                    ImGui::PushID(i+1);
                    if (ImGui::Button("-", ImVec2(ImGui::GetContentRegionAvail().x, 0))) {
                        bindings.erase(bindings.begin() + i);
                    }
                    ImGui::PopID();
                }
                ImGui::EndTable();
            }
        }
        ImGui::EndChild();
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() - ImGui::GetStyle().ItemSpacing.y);
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
        ImGui::InvisibleButton("##hsplitter", ImVec2(FLT_MAX, 8.f));
        ImGui::PopStyleVar();
        if (ImGui::IsItemActive()) {
            can_move_nodes = false;
            height += ImGui::GetIO().MouseDelta.y;
        }

        if (ImGui::BeginChild("second child", ImVec2(0, 0), true)) { ImGui::Text("asdfdf"); }
        ImGui::EndChild();

        ImGui::PopStyleVar();
        ImGui::PopStyleColor();
        return;
    }

    default: throw std::runtime_error{"Unsupported node type"};
    }
}

void RenderGraphGUI::draw_connection_dot(Node *node, bool left, bool center) {
    const auto a    = canvas_start + pan_offset + node->start();
    const auto size = node->size;

    auto dot_pos = a;
    dot_pos.x += left ? 0.f : size.x;
    if (center == false) {
        dot_pos.y += ImGui::GetCursorPosY() - ImGui::GetFrameHeightWithSpacing() * .5f;
        draw_list->AddCircleFilled(convert_vec(dot_pos), 4.f, IM_COL32(66, 245, 152, 255));
    } else {
        dot_pos.y += .5f * (size.y - ImGui::GetFrameHeightWithSpacing()) - 4.f;
        draw_list->AddCircleFilled(convert_vec(dot_pos), 4.f, IM_COL32(66, 245, 152, 255));
    }
}

void RenderGraphGUI::draw_resource_list() {
    ImGui::BeginChild("##Resources", ImVec2(200, 0), true);
    if (ImGui::TreeNode("Resources")) {
        ImGui::Unindent(ImGui::GetTreeNodeToLabelSpacing() * .75f);

        if (ImGui::TreeNode("Buffers")) {
            eng::Engine::instance().renderer_->buffers.for_each([&](const eng::GLBuffer &buff) {
                const auto bid = buff.descriptor.id;
                if (buffer_names.contains(bid) == false) {
                    buffer_names[bid] = std::string{"Buffer"}.append(std::to_string(bid));
                }
                const auto &bname = buffer_names.at(bid);

                bool selected = false;
                ImGui::PushID(bid);
                ImGui::Selectable("label", selected, 0, ImVec2(ImGui::GetContentRegionAvail().x * .75f, 0.f));

                if (ImGui::BeginDragDropSource()) {
                    can_move_nodes = false;

                    ImGui::SetDragDropPayload("GLBuffer", &bid, 4);
                    ImGui::Text(bname.c_str());
                    ImGui::EndDragDropSource();
                }

                ImGui::SameLine();
                ImGui::SetCursorPosY(ImGui::GetCursorPosY() - ImGui::GetStyle().FramePadding.y * .5f);
                ImGui::Button("X", ImVec2(0, ImGui::GetTextLineHeightWithSpacing()));
                ImGui::PopID();
            });

            ImGui::TreePop();
        }
        ImGui::TreePop();
    }
    ImGui::EndChild();
}

void RenderGraphGUI::draw_canvas() {
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(30, 30, 30, 255)); // Set a background color
    if (ImGui::BeginChild(
            "canvas", ImVec2(0.0f, 0.0f), true, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDecoration)) {

        draw_list    = ImGui::GetWindowDrawList();
        canvas_size  = convert_vec(ImGui::GetContentRegionAvail());
        canvas_start = convert_vec(ImGui::GetCursorScreenPos());
        canvas_end   = canvas_start + canvas_size;

        draw_grid();
        draw_nodes();
        handle_panning();
        handle_node_mouse_interaction();
    }
    ImGui::EndChild();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar();
}

bool RenderGraphGUI::mouse_over_node(const Node &n) {
    static constexpr auto corner
        = [](float dx, float dy, float r) { return dx > 0.f || dy > 0.f || dx * dx + dy * dy < r * r; };

    glm::vec2 mouse = convert_vec(ImGui::GetIO().MousePos);
    mouse -= glm::vec2{canvas_start + pan_offset + n.start()};

    const auto w = n.size.x;
    const auto h = n.size.y;
    const auto r = Node::corner_rounding;

    return mouse.x > 0.f && mouse.y > 0.f && mouse.x < w && mouse.y < h && corner(mouse.x - r, mouse.y - r, r)
           && corner(w - mouse.x - r, mouse.y - r, r) && corner(mouse.x - r, h - mouse.y - r, r)
           && corner(w - mouse.x - r, h - mouse.y - r, r);
}

Node NodeBuilder::build(NodeType type) {
    Node node;
    node.type = type;
    switch (type) {
    case NodeType::DepthTest:
        node.size.x                        = 250;
        node.size.y                        = ImGui::GetFrameHeightWithSpacing() * 2.f;
        node.storage.storage["depth_test"] = 0;
        break;

    case NodeType::VAO:
        node.size.x                          = 250;
        node.size.y                          = 500.f;
        node.storage.storage["vao_bindings"] = std::make_any<VaoBindings>();
        /*node.storage.register_data<uint32_t>("binding0", 0);
        node.storage.register_data<uint32_t>("buffer0", 0);*/
        break;
    default: break;
    }
    return node;
}
