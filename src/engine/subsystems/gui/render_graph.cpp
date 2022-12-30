#include "render_graph.hpp"

#include <imgui/imgui.h>

#include "../../engine.hpp"

struct BufferBinding {
    BufferBinding(uint32_t bud, uint32_t bid, uint32_t str, uint32_t off, bool ebo = false)
        : buffer_id{bud}, binding_id{bid}, stride{str}, offset{off}, is_ebo{ebo} {}

    uint32_t buffer_id{0u}, binding_id{0u};
    uint32_t stride{0u}, offset{0u};
    bool is_ebo{false};
};
typedef eng::GLVaoAttributeDescriptor VertexSpec;
typedef std::vector<VertexSpec> VaoAttributes;
typedef std::vector<BufferBinding> VaoBindings;

struct AttrFormatDescriptor {
    eng::GL_FORMAT_ format;
    std::string name;
    uint32_t size_in_bytes;
};

static AttrFormatDescriptor get_format_desc(eng::GL_FORMAT_ f);

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
            if (ImGui::Button("Buffer")) {
                auto buff = eng::Engine::instance().renderer_->buffers.insert(
                    eng::GLBuffer{eng::GLBufferDescriptor{GL_DYNAMIC_STORAGE_BIT}});
                buffer_names.emplace(buff.id, std::string{"Buffer"}.append(std::to_string(buff.id)));
            }
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
        ImGui::PushID(i + 1);

        auto &n = nodes.at(i);

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
        ImGui::PopID();
    }
}

void RenderGraphGUI::draw_node_contents(Node *node) {

    static const char *depth_tests[]{"GL_LESS", "GL_GREATER", "GL_NOTEQUAL"};
    ImGui::PushID(node->id);
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
        break;
    }
    case NodeType::VAO: {

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 5));
        ImGui::PushStyleColor(ImGuiCol_TableRowBgAlt, convert_vec(Colors::grid));

        ImGui::BeginMenuBar();
        ImGui::Text("VAO");
        ImGui::EndMenuBar();

        static std::unordered_map<uint32_t, float> heights;
        if (heights.contains(node->id) == false) heights[node->id] = 150.f;

        auto &vao_bindings = *std::any_cast<VaoBindings>(&node->storage.storage["vao_bindings"]);
        auto &vao_ebo_id   = *std::any_cast<uint32_t>(&node->storage.storage["ebo_id"]);

        if (ImGui::BeginChild("##bindings", ImVec2(0, heights.at(node->id)), true)) {

            if (ImGui::BeginTable("buffer_bindings",
                                  5,
                                  ImGuiTableFlags_ScrollY | ImGuiTableFlags_BordersH | ImGuiTableFlags_RowBg,
                                  ImVec2(0, ImGui::GetContentRegionAvail().y))) {
                ImGui::TableSetupColumn("Buffer");
                ImGui::TableSetupColumn("Binding id");
                ImGui::TableSetupColumn("Stride");
                ImGui::TableSetupColumn("Offset");
                ImGui::TableSetupColumn("");
                ImGui::TableSetupScrollFreeze(0, 1);

                ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
                for (int i = 0; i < 5; ++i) {
                    ImGui::TableSetColumnIndex(i);
                    ImGui::AlignTextToFramePadding();

                    if (i == 4) {
                        if (ImGui::Button("+", ImVec2(ImGui::GetContentRegionAvail().x, 0))) {
                            if (vao_bindings.size() < buffer_names.size()) {
                                auto it = buffer_names.begin();
                                std::advance(it, vao_bindings.size());

                                vao_bindings.emplace_back(it->first, vao_bindings.size(), 0u, 0u, false);
                            }
                        }
                        break;
                    }

                    ImGui::TableHeader(ImGui::TableGetColumnName(i));
                }

                for (auto i = 0u; i < vao_bindings.size(); ++i) {
                    if (buffer_names.contains(vao_bindings.at(i).buffer_id) == false) {
                        vao_bindings.erase(vao_bindings.begin() + i);
                        continue;
                    }
                    ImGui::PushID(i + 1);

                    auto &vao_binding = vao_bindings.at(i);

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);

                    bool s = false;
                    ImGui::Selectable(std::to_string(vao_binding.buffer_id).c_str(),
                                      &s,
                                      ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap,
                                      ImVec2(0, ImGui::GetTextLineHeight() + ImGui::GetStyle().ItemSpacing.y * 1.5f));
                    //=== Node reordering by dragging ===
                    if (ImGui::IsItemActive()) { can_move_nodes = false; }
                    if (ImGui::IsItemActive() && ImGui::IsItemHovered() == false) {
                        uint32_t dst_idx = i + (ImGui::GetMouseDragDelta().y < 0.f ? -1 : 1);
                        ImGui::ResetMouseDragDelta(0);
                        if (dst_idx >= 0u && dst_idx < vao_bindings.size()) {
                            auto temp                = vao_bindings.at(dst_idx);
                            vao_bindings.at(dst_idx) = vao_bindings.at(i);
                            vao_bindings.at(i)       = temp;
                        }
                    }
                    //=== Binding buffer by dragging it from the list ===
                    if (ImGui::BeginDragDropTarget()) {
                        if (auto payload = ImGui::AcceptDragDropPayload("GLBuffer")) {
                            auto data                    = *static_cast<int *>(payload->Data);
                            vao_bindings.at(i).buffer_id = data;
                        }

                        ImGui::EndDragDropTarget();
                    }

                    //=== Context menu ===
                    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10, 5));
                    auto &st = ImGui::GetStyle();
                    if (ImGui::BeginPopupContextItem("buff_name_del_ctx")) {
                        if (ImGui::Selectable("Delete", false)) { vao_bindings.erase(vao_bindings.begin() + i); }
                        ImGui::EndPopup();
                    }
                    ImGui::PopStyleVar();

                    ImGui::TableSetColumnIndex(1);
                    if (ImGui::InputInt("##binding", (int *)&vao_binding.binding_id, 0, 1)) {
                        vao_bindings.at(i).binding_id = glm::max(0, (int)vao_binding.binding_id);
                    }

                    ImGui::TableSetColumnIndex(2);
                    if (ImGui::InputInt("##stride", (int *)&vao_binding.stride, 0, 1)) {
                        vao_bindings.at(i).stride = glm::max(0, (int)vao_binding.stride);
                    }

                    ImGui::TableSetColumnIndex(3);
                    if (ImGui::InputInt("##offset", (int *)&vao_binding.offset, 0, 1)) {
                        vao_bindings.at(i).offset = glm::max(0, (int)vao_binding.offset);
                    }

                    ImGui::TableSetColumnIndex(4);
                    ImGui::BeginGroup();
                    bool checked = vao_ebo_id == vao_binding.buffer_id;
                    if (ImGui::Checkbox("EBO", &checked)) {

                        if (checked) vao_ebo_id = vao_binding.buffer_id;
                        else
                            vao_ebo_id = 0u;
                    }
                    ImGui::EndGroup();

                    ImGui::PopID();
                }

                ImGui::EndTable();
            }
        }
        ImGui::EndChild();
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenOverlapped
                                 | ImGuiHoveredFlags_AllowWhenBlockedByActiveItem)) {
            can_move_nodes = false;
        }

        ImGui::SetCursorPosY(ImGui::GetCursorPosY() - ImGui::GetStyle().WindowPadding.y);
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
        ImGui::InvisibleButton("##hsplitter", ImVec2(FLT_MAX, 8.f));
        ImGui::PopStyleVar();

        if (ImGui::IsItemActive()) {
            can_move_nodes = false;
            heights.at(node->id) += ImGui::GetIO().MouseDelta.y;
            heights.at(node->id) = glm::max(15.f, glm::min(node->size.y, heights.at(node->id)));
        }

        if (ImGui::BeginChild("second child", ImVec2(0, 0), false)) {
            auto &vao_attribs = *std::any_cast<VaoAttributes>(&node->storage.storage["vao_attributes"]);

            if (ImGui::Button("Add attribute", ImVec2(ImGui::GetContentRegionAvail().x, 0))) {
                uint32_t idx, binding{0}, size{0}, offset{0}, normalize{false};
                eng::GL_FORMAT_ format = eng::GL_FORMAT_FLOAT;
                idx                    = vao_attribs.size();

                if (idx > 0) {
                    size   = vao_attribs.back().size;
                    offset = vao_attribs.back().offset + vao_attribs.back().size * 4.f;
                }

                vao_attribs.emplace_back(idx, binding, size, offset, format, normalize);
            }

            if (ImGui::BeginTable("Vertex Specification",
                                  6,
                                  ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersH
                                      | ImGuiTableFlags_SizingFixedFit)) {
                ImGui::TableSetupColumn("Id", 0, ImGui::CalcTextSize("saaas").x);
                ImGui::TableSetupColumn("Binding id", 0, ImGui::CalcTextSize("saaas").x);
                ImGui::TableSetupColumn("Size", 0, ImGui::CalcTextSize("saaas").x);
                ImGui::TableSetupColumn("Offset", 0, ImGui::CalcTextSize("saaas").x);
                ImGui::TableSetupColumn("Type", 0, ImGui::CalcTextSize("GL_UNSIGNED").x);
                ImGui::TableSetupColumn("Normalize", 0, ImGui::CalcTextSize("saaas").x);
                ImGui::TableSetupScrollFreeze(0, 1);

                ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
                for (int i = 0; i < 6; ++i) {
                    ImGui::TableSetColumnIndex(i);
                    ImGui::AlignTextToFramePadding();
                    ImGui::TableHeader(ImGui::TableGetColumnName(i));
                }

                ImGui::PushID("vaos");
                for (auto i = 0u; i < vao_attribs.size(); ++i) {
                    ImGui::PushID(i + 1);
                    auto &attr = vao_attribs.at(i);

                    ImGui::TableNextRow();

                    ImGui::TableSetColumnIndex(0);
                    if (int val = attr.idx; ImGui::InputInt("##Id", &val, 0, 0)) { attr.idx = glm::max(0, val); }
                    ImGui::TableSetColumnIndex(1);
                    if (int val = attr.binding; ImGui::InputInt("##Binding id", &val, 0, 0)) {
                        attr.binding = glm::max(0, val);
                    }
                    ImGui::TableSetColumnIndex(2);
                    if (int val = attr.size; ImGui::InputInt("##Size", &val, 0, 0)) { attr.size = glm::max(0, val); }
                    ImGui::TableSetColumnIndex(3);
                    if (int val = attr.offset; ImGui::InputInt("##Offset", &val, 0, 0)) {
                        attr.offset = glm::max(0, val);
                    }
                    ImGui::TableSetColumnIndex(4);
                    {
                        eng::GL_FORMAT_ all_formats[]{eng::GL_FORMAT_FLOAT};
                        if (ImGui::BeginListBox(
                                "##Format",
                                ImVec2(ImGui::CalcTextSize("GL_UNSIGNED").x, ImGui::GetFrameHeightWithSpacing()))) {
                            for (int i = 0; i < IM_ARRAYSIZE(all_formats); ++i) {
                                auto f    = eng::GL_FORMAT_(i);
                                auto desc = get_format_desc(f);

                                if (ImGui::Selectable(desc.name.c_str(), eng::GL_FORMAT_(i) == attr.gl_format)) {
                                    attr.gl_format = f;
                                }
                            }
                            ImGui::EndListBox();
                        }
                    }
                    ImGui::TableSetColumnIndex(5);
                    if (bool val = attr.normalize; ImGui::Checkbox("##Normalize", &val)) { attr.normalize = val; }

                    ImGui::PopID();
                }
                ImGui::PopID();
                ImGui::EndTable();
            }
        }
        ImGui::EndChild();
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenOverlapped
                                 | ImGuiHoveredFlags_AllowWhenBlockedByActiveItem)) {
            can_move_nodes = false;
        }

        ImGui::PopStyleVar();
        ImGui::PopStyleColor();
        draw_connection_dot(node, false, true);
        break;
    }

    default: throw std::runtime_error{"Unsupported node type"};
    }

    ImGui::PopID();
}

void RenderGraphGUI::draw_connection_dot(Node *node, bool left, bool center) {
    const auto a    = canvas_start + pan_offset + node->start();
    const auto size = node->size;

    auto draw_list = ImGui::GetForegroundDrawList();
    draw_list->PushClipRect(convert_vec(canvas_start), convert_vec(canvas_end));
    auto dot_pos = a;
    dot_pos.x += left ? 0.f : size.x;
    if (center == false) {
        dot_pos.y += ImGui::GetCursorPosY() - ImGui::GetFrameHeightWithSpacing() * .5f;
        draw_list->AddCircleFilled(convert_vec(dot_pos), 4.f, IM_COL32(66, 245, 152, 255));
    } else {
        dot_pos.y += .5f * (size.y - ImGui::GetFrameHeightWithSpacing()) - 4.f;
        draw_list->AddCircleFilled(convert_vec(dot_pos), 4.f, IM_COL32(66, 245, 152, 255));
    }
    
    draw_list->PopClipRect();
}

void RenderGraphGUI::draw_resource_list() {
    static bool selected;
    static uint32_t sel_id;
    static bool edit = false;

    ImGui::BeginChild("##Resources", ImVec2(200, 0), true);

    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::TreeNode("Resources")) {
        ImGui::Unindent(ImGui::GetTreeNodeToLabelSpacing() * .75f);

        ImGui::SetNextItemOpen(true, ImGuiCond_Once);
        if (ImGui::TreeNode("Buffers")) {

            uint32_t buff_to_delete{0u};
            for (const auto &[id, name] : buffer_names) {
                const auto &bname = name;

                ImGui::PushID(id);

                const auto label    = bname.c_str();
                const auto sel_cond = selected && id == sel_id;
                const auto flags    = ImGuiSelectableFlags_AllowDoubleClick;
                const auto size     = ImVec2(ImGui::GetContentRegionAvail().x * .75f, 0.f);
                if (id != sel_id || !edit)
                    if (ImGui::Selectable(label, sel_cond, flags, size)) {
                        if (selected) { edit = false; }
                        selected = true;
                        sel_id   = id;
                        if (ImGui::IsMouseDoubleClicked(0)) { edit = true; }
                    }
                if (ImGui::BeginPopupContextItem()) {
                    ImGui::Selectable("Buffer editor");
                    ImGui::EndPopup();
                }
                if (edit == true && id == sel_id) {
                    static char buff[512];
                    memcpy(buff, bname.c_str(), bname.size() + 1);

                    if (ImGui::IsMouseDoubleClicked(0)) { ImGui::SetKeyboardFocusHere(0); }

                    if (ImGui::InputText("##text",
                                         buff,
                                         512,
                                         ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CharsNoBlank)) {
                        buffer_names.at(id) = buff;
                        edit                = false;
                        selected            = false;
                    }
                }

                if (ImGui::BeginDragDropSource()) {
                    can_move_nodes = false;

                    ImGui::SetDragDropPayload("GLBuffer", &id, 4);
                    ImGui::Text(bname.c_str());
                    ImGui::EndDragDropSource();
                }

                ImGui::SameLine();
                ImGui::SetCursorPosY(ImGui::GetCursorPosY() - ImGui::GetStyle().FramePadding.y * .5f);
                if (ImGui::Button("X", ImVec2(0, ImGui::GetTextLineHeightWithSpacing()))) { buff_to_delete = id; }
                ImGui::PopID();
            }

            if (buff_to_delete > 0u) { buffer_names.erase(buff_to_delete); }

            ImGui::TreePop();
        }
        if(ImGui::TreeNode("Vaos")) {
            ImGui::TreePop();
        }
        ImGui::TreePop();
    }
    ImGui::EndChild();

    if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem) == false && ImGui::IsMouseClicked(0)) {
        selected = false;
        edit     = false;
    }
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

        handle_panning();
        handle_node_mouse_interaction();
        can_move_nodes = true;
        draw_grid();
        draw_nodes();
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
        node.size.x                            = 300;
        node.size.y                            = 450.f;
        node.storage.storage["vao_bindings"]   = std::make_any<VaoBindings>();
        node.storage.storage["vao_attributes"] = std::make_any<VaoAttributes>();
        node.storage.storage["ebo_id"]         = 0u;
        break;
    default: break;
    }
    return node;
}

static AttrFormatDescriptor get_format_desc(eng::GL_FORMAT_ f) {
    switch (f) {
    case eng::GL_FORMAT_FLOAT:
        return AttrFormatDescriptor{.format = f, .name = "GL_Float", .size_in_bytes = sizeof(1.f)};

    default: assert(false && "unsupported format type");
    }
}