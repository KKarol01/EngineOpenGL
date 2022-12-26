#include "render_graph.hpp"

#include "../../engine.hpp"
#include <imgui/imgui.h>

static glm::vec2 convert_vec(ImVec2 v) { return {v.x, v.y}; }
static ImVec2 convert_vec(glm::vec2 v) { return {v.x, v.y}; }
static unsigned convert_vec(glm::u8vec4 v) { return IM_COL32(v.r, v.g, v.b, v.a); }

void RenderGraphGUI::draw() {

    static auto r = 5.f;
    static auto w = 160.f, h = 40.f;

    ImGui::Begin("render graph", &open);
    ImGui::BeginChild(
        "node settings", ImVec2(0, ImGui::GetTextLineHeightWithSpacing() * 4.f), true, ImGuiWindowFlags_NoMove);
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

    ImGui::BeginChild("render-graph-buttons",
                      ImVec2(0.f, ImGui::GetTextLineHeightWithSpacing() * 2.f),
                      true,
                      ImGuiWindowFlags_NoScrollbar);
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
    ImGui::EndChild();

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(30, 30, 30, 255)); // Set a background color
    ImGui::BeginChild("canvas", ImVec2(0.0f, 0.0f), true, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDecoration);
    ImGui::PopStyleVar();

    draw_list    = ImGui::GetWindowDrawList();
    canvas_size  = convert_vec(ImGui::GetContentRegionAvail());
    canvas_start = convert_vec(ImGui::GetCursorScreenPos());
    canvas_end   = canvas_start + canvas_size;

    handle_panning();
    handle_node_mouse_interaction();
    node_input_focus = false;
    draw_grid();
    draw_nodes();

    ImGui::EndChild();
    ImGui::PopStyleColor();
    ImGui::End();
}

void RenderGraphGUI::handle_panning() {
    auto &io = ImGui::GetIO();
    if (ImGui::IsMouseDragging(1, 0.5f) == false) return;
    pan_offset += convert_vec(io.MouseDelta);
}

void RenderGraphGUI::handle_node_mouse_interaction() {
    if (node_input_focus) return;

    Node *tnode{nullptr};
    uint32_t k       = 0;
    bool dont_update = false;
    for (auto i = 0u; i < nodes.size(); ++i) {
        auto &n = nodes.at(i);

        if (mouse_over_node(n)) {
            n.mouse_hovering = false;
            if (dont_update) continue;
            tnode = &n;
            k     = i;
        } else if (n.mouse_dragging && dont_update == false) {
            tnode       = &n;
            k           = i;
            dont_update = true;
        } else {
            n.mouse_hovering = false;
            n.mouse_down     = false;
            n.mouse_dragging = false;
        }
    }

    if (inode == nullptr || (inode && inode->mouse_dragging == false)) inode = tnode;
    if (inode == nullptr) return;

    inode->mouse_hovering = true;
    inode->mouse_down     = ImGui::IsMouseDown(0);

    auto is_dragging = ImGui::IsMouseDragging(0);

    if (inode->mouse_dragging == false && is_dragging) {
        inode->drag_start = inode->drag_end = convert_vec(ImGui::GetIO().MousePos);
        inode->mouse_dragging               = true;
    } else if (is_dragging) {
        inode->drag_end = convert_vec(ImGui::GetIO().MousePos);
        inode->position += inode->drag_end - inode->drag_start;
        inode->drag_start = inode->drag_end;
    } else {
        inode->mouse_dragging = false;
    }

    /*if (inode->mouse_down) {
        Node temp    = std::move(nodes.back());
        nodes.back() = std::move(*inode);
        nodes.at(k)  = std::move(temp);
        inode        = &nodes.back();
    }*/
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

        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, Node::corner_rounding);

        ImGui::PushStyleColor(ImGuiCol_MenuBarBg, IM_COL32(252, 80, 68, 255));
        if (n.mouse_hovering) ImGui::PushStyleColor(ImGuiCol_Border, convert_vec(Colors::node_hovered));
        if (n.mouse_down || n.mouse_dragging) ImGui::PushStyleColor(ImGuiCol_Border, convert_vec(Colors::node_clicked));

        ImGui::SetCursorPos(convert_vec(pan_offset + n.position));
        if (ImGui::BeginChild(i + 1, ImVec2(n.size.x, n.size.y), true, ImGuiWindowFlags_MenuBar)) {
            draw_node_contents(&n);
        }
        ImGui::EndChild();

        ImGui::PopStyleColor();
        if (n.mouse_hovering) ImGui::PopStyleColor();
        if (n.mouse_down || n.mouse_dragging) ImGui::PopStyleColor();
        ImGui::PopStyleVar();
    }
}

void RenderGraphGUI::draw_node_contents(Node *node) {

    static const char *depth_tests[]{"GL_LESS", "GL_GREATER", "GL_NOTEQUAL"};

    switch (node->type) {
    case NodeType::DepthTest: {
        ImGui::BeginMenuBar();
        ImGui::Text("TITLE");
        ImGui::EndMenuBar();

        auto &depth_test = *node->storage.get<uint32_t>("depth_test");
        ImGui::PushItemWidth(ImGui::CalcTextSize("GL_NOTEQUAL").x + 25.f);
        if (ImGui::BeginCombo("Depth test", depth_tests[depth_test])) {
            node_input_focus = true;

            for (auto i = 0u; i < IM_ARRAYSIZE(depth_tests); ++i) {
                if (ImGui::Selectable(depth_tests[i], i == depth_test)) { depth_test = i; }
            }
            ImGui::EndCombo();
        }
        draw_connection_dot(node);
        ImGui::PopItemWidth();
        return;
    }
    default: break;
    }
}

void RenderGraphGUI::draw_connection_dot(Node *node) {
    const auto a    = canvas_start + pan_offset + node->start();
    const auto size = node->size;

    draw_list->AddCircleFilled(
        convert_vec(a + glm::vec2{size.x, ImGui::GetCursorPosY() - ImGui::GetFrameHeightWithSpacing() * .5f}),
        4.f,
        IM_COL32(66, 245, 152, 255));
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
        node.storage.register_data<int>("depth_test", 0);
        node.size.x = 250;
        node.size.y = ImGui::GetFrameHeightWithSpacing() * 2.5f;
        break;
    default: break;
    }
    return node;
}
