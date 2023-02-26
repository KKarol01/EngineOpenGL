#include "render_graph.hpp"

#include <algorithm>

#include <imgui/imgui.h>
#include <iterator>

#include "../../engine.hpp"

RenderGraphGUI::RenderGraphGUI() {
    buffers_names[0] = "empty";
    add_node(NodeType::VAO);
    add_node(NodeType::Stage);
    // add_node(NodeType::Stage);
}

constexpr static auto gvec2(ImVec2 v) { return glm::vec2{v.x, v.y}; }
constexpr static auto ivec2(glm::vec2 v) { return ImVec2{v.x, v.y}; }
constexpr static auto icol32(glm::u8vec4 v) { return IM_COL32(v.r, v.g, v.b, v.a); }

constexpr static auto linear_bezier    = [](float t, glm::vec2 a, glm::vec2 b) { return (1.f - t) * a + t * b; };
constexpr static auto quadratic_bezier = [](float t, glm::vec2 a, glm::vec2 b, glm::vec2 c) {
    return linear_bezier(t, linear_bezier(t, a, b), linear_bezier(t, b, c));
};
constexpr static auto cubic_bezier = [](float t, glm::vec2 a, glm::vec2 b, glm::vec2 c, glm::vec2 d) {
    return linear_bezier(t, quadratic_bezier(t, a, b, c), quadratic_bezier(t, b, c, d));
};

static void draw_cubic_bezier(
    ImDrawList *list, glm::vec2 a, glm::vec2 b, glm::vec2 c, glm::vec2 d, glm::u8vec4 c1, glm::u8vec4 c2) {
    const float approx_length = glm::distance(a, d);
    const float step          = 1.f / glm::max<float>(100.f, approx_length);

    auto prev = cubic_bezier(0.f, a, b, c, d);
    for (float t = step; t <= 1.f; t += step) {
        const auto curr  = cubic_bezier(t, a, b, c, d);
        const auto color = glm::mix(c1, c2, t);
        list->AddLine(ivec2(prev), ivec2(curr), icol32(color), 2.f);

        prev = curr;
    }
}

static void draw_cubic_bezier(ImDrawList *list, glm::vec2 a, glm::vec2 b, glm::vec2 c, glm::vec2 d, glm::u8vec4 c1) {
    draw_cubic_bezier(list, a, b, c, d, c1, c1);
}

constexpr static auto cubic_solver_real = [](float A, float B, float C, float D) {
    // p and q are depressed form factors.
    constexpr auto E = glm::epsilon<float>();
    const auto p     = (3.f * A * C - B * B) / (3.f * A * A);
    const auto q     = (2.f * B * B * B - 9.f * A * B * C + 27.f * A * A * D) / (27.f * A * A * A);

    std::vector<float> roots;
    if (fabsf(p) < E) { // if p or q are zero, then problem reduces to quadratic form
        roots.push_back(cbrtf(-q));
    } else if (fabsf(q) < E) {
        roots.push_back(0.f);

        if (p < 0.f) {
            const auto psqrt = sqrt(-p);
            roots.push_back(psqrt);
            roots.push_back(-psqrt);
        }
    } else {
        constexpr auto oneOverThree = 0.3333333f;
        constexpr auto PI           = glm::pi<float>();
        const auto delta            = q * q * .25f + p * p * p * .037;
        const auto deltasqrt        = sqrtf(delta);
        const auto QoverP           = q / p;
        const auto negHalfQ         = -.5f * q;

        if (delta > 0.f) { // one real root - cardano's formula
            roots.push_back(cbrtf(negHalfQ + deltasqrt) + cbrtf(negHalfQ - deltasqrt));
        } else if (fabsf(delta) < E) { // two roots (one is a multiple)
            roots.push_back(-1.5f * QoverP);
            roots.push_back(3.f * QoverP);
        } else { // three roots, using trigonometric solution
            const auto u = 2.f * sqrtf(-p * oneOverThree);
            const auto t = acosf(3.f * QoverP / u) * oneOverThree;
            const auto k = 2.f * PI * oneOverThree;
            roots.push_back(u * cosf(t));
            roots.push_back(u * cosf(t - k));
            roots.push_back(u * cosf(t - 2.f * k));
        }
    }

    const auto fromDepressed = B / (3.f * A);
    for (auto &r : roots) {
        r -= fromDepressed; // bring back root(s) from depressed form
    }

    return roots;
};

static bool line_cubic_bezier(glm::vec2 l0, glm::vec2 l1, glm::vec2 c0, glm::vec2 c1, glm::vec2 c2, glm::vec2 c3) {
    // Map a P point that belongs to the line and the curve
    // using gradient equality due to colinearity.
    const auto ld = l0.x * (l1.y - l0.y) + l0.y * (l0.x - l1.x);
    const auto v  = glm::vec2{l1.y - l0.y, l0.x - l1.x};
    // A B C D are the cubic coefficients
    const auto A = glm::dot(v, -c0 + 3.f * c1 - 3.f * c2 + c3);
    const auto B = glm::dot(v, 3.f * c0 - 6.f * c1 + 3.f * c2);
    const auto C = glm::dot(v, -3.f * c0 + 3.f * c1);
    const auto D = glm::dot(v, c0) - ld;

    const auto qr = cubic_solver_real(A, B, C, D);
    for (auto &r : qr) {
        if (r >= 0.f && r <= 1.f) {
            const auto P = cubic_bezier(r, c0, c1, c2, c3);
            if (glm::length(P - l0) <= glm::length(l0 - l1)) { return true; }
        }
    }

    return false;
}

static void draw_menu_bar(const char *title, Node *node, bool foldable = true);

void RenderGraphGUI::draw() {
    bgdraw_list = ImGui::GetBackgroundDrawList();

    enable_node_interaction();

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0));
    if (ImGui::Begin("render graph", &open)) {
        window_start = gvec2(ImGui::GetCursorScreenPos()) - gvec2(ImGui::GetStyle().WindowPadding);
        window_size  = gvec2(ImGui::GetContentRegionAvail()) + 2.f * gvec2(ImGui::GetStyle().WindowPadding);

        draw_background();

        if (line_dragging) {
            disable_node_interaction();

            float dist   = glm::distance(node_connection.line_dragging_start, gvec2(ImGui::GetMousePos()));
            float offset = glm::mix(50.f, 200.f, dist / glm::length(canvas_size) * 3.f);

            const auto p1
                = canvas_start + get_node(node_connection.from)->position + node_connection.line_dragging_start;
            const auto p4 = gvec2(ImGui::GetMousePos());
            const auto p2 = p1 + glm::vec2{offset, 0.f};
            const auto p3 = p4 - glm::vec2{offset, 0.f};

            bgdraw_list->AddBezierCubic(ivec2(p1), ivec2(p2), ivec2(p3), ivec2(p4), IM_COL32(255, 0, 0, 255), 3.f);
        }
        if (draw_line) {
            float dist   = glm::distance(node_connection.line_dragging_start, node_connection.line_dragging_end);
            float offset = glm::mix(50.f, 200.f, dist / glm::length(canvas_size) * 3.f);

            const auto p1
                = canvas_start + get_node(node_connection.from)->position + node_connection.line_dragging_start;
            const auto p4 = canvas_start + get_node(node_connection.to)->position + node_connection.line_dragging_end;
            const auto p2 = p1 + glm::vec2{offset, 0.f};
            const auto p3 = p4 - glm::vec2{offset, 0.f};

            bgdraw_list->AddBezierCubic(ivec2(p1), ivec2(p2), ivec2(p3), ivec2(p4), IM_COL32(255, 0, 0, 255), 3.f);
        }

        if (ImGui::BeginChild("buffer list", ImVec2(250, 0), true, ImGuiWindowFlags_NoMove)) { draw_buffer_list(); }
        ImGui::EndChild();

        ImGui::SameLine();

        if (ImGui::BeginChild("canvas", ImVec2(0, 0), true, ImGuiWindowFlags_NoMove)) {
            canvas_start = gvec2(ImGui::GetCursorScreenPos()) - gvec2(ImGui::GetStyle().WindowPadding);
            canvas_size  = gvec2(ImGui::GetContentRegionAvail());
            draw_canvas();
        }
        ImGui::EndChild();
    }
    ImGui::End();
    ImGui::PopStyleColor();

    static auto is_mouse_down = false;
    if (ImGui::IsAnyItemHovered() || is_mouse_down) {

        if (is_mouse_down == false && ImGui::IsMouseClicked(0)) {
            is_mouse_down = true;
        } else if (ImGui::IsMouseReleased(0)) {
            is_mouse_down = false;
        }

        disable_node_interaction();
    }
    if (line_dragging && ImGui::IsMouseReleased(0) == true) { line_dragging = false; }

    mouse_node_interactions();
}

void RenderGraphGUI::draw_nodes() {}

void RenderGraphGUI::draw_node_contents(Node *node) {

    switch (node->type) {
    case NodeType::VAO: {
        draw_menu_bar("VAO", node);

        if (ImGui::BeginChild("content")) {
            ImGui::PushStyleColor(ImGuiCol_TableRowBgAlt, icol32(glm::vec4{Colors::node} * 1.55f));

            auto &table_height = *std::any_cast<float>(&node->storage.at("bindings_table_height"));
            auto &desc         = *std::any_cast<eng::GLVaoDescriptor>(&node->storage.at("descriptor"));
            auto &bindings     = desc.buff_bindings;
            eng::GLVaoBufferBinding *binding_to_delete = nullptr;

            draw_connection_button(node, "Vao >", true);

            if (ImGui::Button("Add binding", ImVec2(ImGui::GetContentRegionMax().x, 0))) {
                bindings.emplace_back();
                bindings.back().binding = bindings.size() - 1;
            }

            if (ImGui::BeginTable("vao_bindings_table",
                                  4,
                                  ImGuiTableFlags_BordersH | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY,
                                  ImVec2(0, table_height))) {

                ImGui::TableSetupColumn("binding");
                ImGui::TableSetupColumn("buffer");
                ImGui::TableSetupColumn("stride");
                ImGui::TableSetupColumn("offset");
                ImGui::TableSetupScrollFreeze(0, 1);
                ImGui::TableHeadersRow();

                for (int i = 0; i < bindings.size(); ++i) {
                    auto &b = bindings[i];

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    if (ImGui::Selectable(std::to_string(b.binding).c_str(),
                                          false,
                                          ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap,
                                          ImVec2(0, ImGui::GetFrameHeight()))) {}
                    if (ImGui::BeginPopupContextItem(NULL)) {
                        if (ImGui::Button("Delete")) { binding_to_delete = &b; }
                        ImGui::EndPopup();
                    }

                    if (ImGui::IsItemActive() && ImGui::IsItemHovered() == false) {
                        int dir = ImGui::GetMouseDragDelta(0).y > 0.f ? 1 : -1;

                        if ((i + dir) >= 0 && i + dir < bindings.size()) {
                            auto temp            = bindings.at(i);
                            bindings.at(i)       = bindings.at(i + dir);
                            bindings.at(i + dir) = temp;
                            ImGui::ResetMouseDragDelta();
                        }
                    }
                    ImGui::PushID(node->id);
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text(buffers_names.at(b.buffer).c_str());
                    if (ImGui::BeginDragDropTarget()) {
                        if (auto payload = ImGui::AcceptDragDropPayload("vao_binding")) {
                            b.buffer = *static_cast<uint32_t *>(payload->Data);
                        }
                        ImGui::EndDragDropTarget();
                    }

                    ImGui::TableSetColumnIndex(2);
                    if (int input = b.stride; ImGui::InputInt("##stride", &input, 0, 0)) {
                        b.stride = glm::max(0, input);
                    }
                    ImGui::TableSetColumnIndex(3);
                    if (int input = b.offset; ImGui::InputInt("##offset", &input, 0, 0)) {
                        b.offset = glm::max(0, input);
                    }
                    ImGui::PopID();
                }
                ImGui::EndTable();
            }

            if (binding_to_delete) {
                for (auto i = 0u; i < bindings.size(); ++i) {
                    if (bindings[i].binding == binding_to_delete->binding) {
                        bindings.erase(bindings.begin() + i);
                        binding_to_delete = nullptr;
                        break;
                    }
                }

                assert(("Could not find the binding to delete" && binding_to_delete == nullptr));
            }

            ImGui::SetCursorPosY(ImGui::GetCursorPosY() - ImGui::GetStyle().ItemSpacing.y);
            if (ImGui::Button("slider", ImVec2(FLT_MAX, 10.f))) {}
            if (ImGui::IsItemActive()) {
                allow_node_interaction = false;
                table_height += ImGui::GetMouseDragDelta(0, 0.f).y;
                ImGui::ResetMouseDragDelta();
            }
            if (ImGui::Button("Add attribute", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetFrameHeight()))) {
                desc.attributes.emplace_back(desc.attributes.size() > 0 ? desc.attributes.back().idx + 1 : 0, 0, 0, 0);
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

                eng::GLVaoAttributeDescriptor *attribute_to_delete = nullptr;
                for (auto i = 0u; i < desc.attributes.size(); ++i) {
                    auto &attrib = desc.attributes.at(i);
                    ImGui::TableNextRow();

                    ImGui::PushID(i + 1);

                    ImGui::TableSetColumnIndex(0);
                    {
                        auto c = ImGui::GetCursorPos();
                        ImGui::Selectable(("##" + std::to_string(attrib.idx)).c_str(),
                                          false,
                                          ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap,
                                          ImVec2(0, ImGui::GetFrameHeight()));
                        if (ImGui::BeginPopupContextItem()) {
                            if (ImGui::Button("Delete")) { attribute_to_delete = &attrib; }
                            ImGui::EndPopup();
                        }
                        ImGui::SetCursorPos(c);
                        if (int input = attrib.idx; ImGui::InputInt("##id", &input, 0, 0)) {
                            attrib.idx = fmaxf(0, input);
                        }
                    }

                    ImGui::TableSetColumnIndex(1);
                    if (int input = attrib.binding; ImGui::InputInt("##binding", &input, 0, 0)) {
                        attrib.binding = fmaxf(0, input);
                    }

                    ImGui::TableSetColumnIndex(2);
                    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                    if (int input = attrib.size; ImGui::InputInt("##size", &input, 0, 0)) {
                        attrib.size = fmaxf(0, input);
                    }

                    ImGui::TableSetColumnIndex(3);
                    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                    if (int input = attrib.offset; ImGui::InputInt("##offset", &input, 0, 0)) {
                        attrib.offset = fmaxf(0, input);
                    }

                    ImGui::TableSetColumnIndex(4);
                    const char *types[]{"GL_FLOAT"};
                    assert(("Not all attribute types are supported in render_graph"
                            && IM_ARRAYSIZE(types) == eng::GL_FORMAT_FLOAT + 1));
                    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                    if (ImGui::BeginCombo("##gl_type", types[(int)attrib.gl_format])) {
                        for (int i = 0; i < IM_ARRAYSIZE(types); ++i) {
                            if (ImGui::Selectable(types[i])) { attrib.gl_format = eng::GL_FORMAT_(i); }
                        }

                        ImGui::EndCombo();
                    }

                    ImGui::TableSetColumnIndex(5);
                    ImGui::Checkbox("##normalize", &attrib.normalize);
                    ImGui::PopID();
                }

                if (attribute_to_delete) {
                    for (auto i = 0u; i < desc.attributes.size(); ++i) {
                        if (desc.attributes[i].idx == attribute_to_delete->idx) {
                            desc.attributes.erase(desc.attributes.begin() + i);
                            attribute_to_delete = nullptr;
                            break;
                        }
                    }

                    assert(("Could not find attribute to delete" && attribute_to_delete == nullptr));
                }

                ImGui::EndTable();
            }
            ImGui::PopStyleColor();
        }
        ImGui::EndChild();
        break;
    }

    case NodeType::Stage: {
        draw_menu_bar("Stage", node);

        if (ImGui::BeginChild("content")) { draw_connection_button(node, "> Vao", false); }

        ImGui::EndChild();
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
        n.storage["descriptor"]            = eng::GLVaoDescriptor{};
        n.storage["bindings_table_height"] = n.size.y * .45f;
        break;

    case NodeType::Stage: n.size = glm::vec2{400.f, 500.f};

    default: break;
    }

    n.min_size = .05f * n.size;
    n.max_size = 1.5f * n.size;
    nodes.push_back(std::move(n));
}

Node *RenderGraphGUI::get_node(NodeID id) {
    for (auto it = nodes.begin(); it != nodes.end(); ++it) {
        if (it->id == id) return &*it;
    }

    return nullptr;
}

void RenderGraphGUI::mouse_node_interactions() {
    if (allow_node_interaction == false) { return; }

    // Helper function - find node based on predicate
    static const auto find_node = [this](std::function<bool(Node *)> f) -> Node * {
        for (auto i = nodes.size() - 1;; --i) {
            if (f(&nodes.at(i))) return &nodes.at(i);

            if (i == 0u) break;
        }
        return nullptr;
    };

    const auto is_canvas_hovered
        = ImGui::IsMouseHoveringRect(ivec2(canvas_start), ivec2(canvas_start + canvas_size), false);

    if (is_canvas_hovered == false) {
        active_node_id     = 0u;
        active_node_action = NodeAction_None;
        ImGui::SetMouseCursor(ImGuiMouseCursor_Arrow);
        return;
    }

    if (active_node_id == 0u || active_node_action == NodeAction_Hovering) {
        auto hovered_node = find_node([&](Node *n) {
            auto p = n->position;
            p += canvas_start;
            return ImGui::IsMouseHoveringRect(ivec2(p), ivec2(p + n->size), false);
        });

        if (hovered_node == nullptr) { return; }

        active_node_id     = hovered_node->id;
        active_node_action = NodeAction_Hovering;
    }

    auto node = get_node(active_node_id);
    assert(node);

    if (ImGui::IsMouseClicked(0)) { move_node_to_front(active_node_id); }

    if (active_node_action == NodeAction_Hovering) {
        const auto mp  = gvec2(ImGui::GetMousePos());
        auto node_edge = node->position + node->size;
        node_edge += canvas_start;

        ImGui::SetMouseCursor(ImGuiMouseCursor_Arrow);
        if (fabsf(mp.x - node_edge.x) < 10.f) {
            ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);

            if (ImGui::IsMouseClicked(0)) { active_node_action = NodeAction_Resizing; }
        } else if (ImGui::IsMouseDown(0)) {
            active_node_action = NodeAction_Down;

            if (ImGui::IsMouseDragging(0, .0f)) { active_node_action = NodeAction_Dragging; }
        }
    }

    if (active_node_action == NodeAction_Resizing) {
        node->size.x += ImGui::GetMouseDragDelta(0, 0.f).x;
        ImGui::ResetMouseDragDelta();
        ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
    } else if (active_node_action == NodeAction_Dragging) {
        node->position += gvec2(ImGui::GetMouseDragDelta(0, 0.f));
        ImGui::ResetMouseDragDelta();
    }

    if (ImGui::IsMouseReleased(0)) {
        active_node_id     = 0u;
        active_node_action = NodeAction_None;
        ImGui::SetMouseCursor(ImGuiMouseCursor_Arrow);
    }
}

void RenderGraphGUI::draw_buffer_list() {
    if (ImGui::BeginChild("resource_view", ImVec2(200.f, 0.f), false, ImGuiWindowFlags_NoMove)) {
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
                        allow_node_interaction = false;
                        resource_dragging      = true;
                        ImGui::SetDragDropPayload("vao_binding", &id, 4);
                        ImGui::Text(name.c_str());
                        ImGui::EndDragDropSource();
                    }

                    if (resource_dragging && ImGui::IsMouseDown(0) == false) {
                        resource_dragging      = false;
                        allow_node_interaction = true;
                    }
                }

                ImGui::PopID();
            }

            ImGui::TreePop();
        }
    }
    ImGui::EndChild();
}

void RenderGraphGUI::draw_canvas() {
    ImGui::PushStyleColor(ImGuiCol_ChildBg, icol32(Colors::node));

    for (auto &n : nodes) {
        ImGui::SetCursorPos(ivec2(n.position));

        if (ImGui::BeginChild(n.id, ivec2(n.size), true, ImGuiWindowFlags_MenuBar)) { draw_node_contents(&n); }
        ImGui::EndChild();
    }

    ImGui::PopStyleColor();
}

void RenderGraphGUI::move_node_to_front(NodeID id) {
    if (nodes.size() <= 1) { return; }
    if (nodes.at(nodes.size() - 1).id == id) { return; }

    auto it = std::find_if(nodes.begin(), nodes.end(), [id](const auto &n) { return id == n.id; });
    assert(it != nodes.end());

    std::rotate(it, it + 1, nodes.end());
}

void RenderGraphGUI::start_connection() {}

void RenderGraphGUI::draw_connection_button(Node *node, const char *name, bool is_output) {
    const auto cx = ImGui::GetCursorPosX();
    const auto cr = ImGui::GetContentRegionAvail().x;
    const auto ts = ImGui::CalcTextSize("a").x * (strlen(name) + 2); // +2 for padding

    glm::vec2 connection_button_start, connection_button_end;
    glm::vec2 connection_point;

    ImGui::SetCursorPosX(cx + is_output ? (cr - ts) : 0.f);
    connection_button_start = gvec2(ImGui::GetCursorPos());
    if (ImGui::Button(name, ImVec2(ts, 0.f))) {}
    connection_button_end = glm::vec2{connection_button_start.x + ts, ImGui::GetFrameHeightWithSpacing()};

    connection_point = connection_button_start + 0.5f * (connection_button_end - connection_button_start);
    connection_point += gvec2({0.f, ImGui::GetFrameHeightWithSpacing()});

    if (is_output && line_dragging == false && ImGui::IsItemActive()) {
        line_dragging                       = true;
        node_connection.from                = node->id;
        node_connection.line_dragging_start = connection_point;

    } else if (is_output == false && line_dragging
               && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem)) {
        if (ImGui::IsMouseReleased(0)) {
            draw_line                         = true;
            line_dragging                     = false;
            node_connection.to                = node->id;
            node_connection.line_dragging_end = connection_point;
        }
    }
}

void RenderGraphGUI::draw_background() {
    bgdraw_list->AddRectFilled(ivec2(window_start), ivec2(window_start + window_size), icol32(Colors::window));
}

static void draw_menu_bar(const char *title, Node *node, bool foldable) {
    if (ImGui::BeginMenuBar()) {
        std::string arrow_id = title;
        arrow_id += "_fold_btn";

        if (foldable) {
            if (ImGui::ArrowButton(arrow_id.c_str(), node->opened ? ImGuiDir_Down : ImGuiDir_Right)) {
                node->opened = !node->opened;
            }
        }

        ImGui::Text(title);
        ImGui::EndMenuBar();
    }
}
