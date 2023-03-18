#pragma once

#include <vector>
#include <string>
#include <memory>
#include <any>
#include <unordered_map>
#include <functional>
#include <compare>

#include <glm/glm.hpp>

#include "../../engine/types/types.hpp"

struct ImDrawList;
typedef uint32_t NodeID;
enum class NodeType {
    None,
    PipelineBegin,
    Stage,
    DepthTest,
    VAO,
};

struct NodeIO {
    NodeIO(NodeID origin, NodeID target, glm::vec2 line_start, glm::vec2 line_end)
        : origin{origin}, target{target}, line_start{line_start}, line_end{line_end} {}

    auto operator<=>(const NodeIO &o) const {
        if (auto cmp = origin <=> o.origin; cmp != 0) { return cmp; }
        return target <=> o.target;
    }
    auto operator==(const NodeIO &o) const { return origin == o.origin && target == o.target; }

    NodeID origin{0}, target{0};
    glm::vec2 line_start, line_end;
};

struct Node {
  public:
    Node() { id = gid++; }
    Node(const Node &n) noexcept { *this = n; }
    Node(Node &&n) noexcept { *this = std::move(n); }
    Node &operator=(const Node &n) noexcept {
        id       = n.id;
        type     = n.type;
        position = n.position;
        size     = n.size;
        min_size = n.min_size;
        max_size = n.max_size;
        storage  = n.storage;
        opened   = n.opened;

        return *this;
    }
    Node &operator=(Node &&n) noexcept {
        id       = std::move(n.id);
        type     = std::move(n.type);
        position = std::move(n.position);
        size     = std::move(n.size);
        min_size = std::move(n.min_size);
        max_size = std::move(n.max_size);
        storage  = std::move(n.storage);
        opened   = std::move(n.opened);

        return *this;
    }
    ~Node() = default;

    auto start() const { return position; }
    auto end() const { return position + size; }
    auto set_position(glm::vec2 npos) { position = npos; }
    auto set_size(glm::vec2 nsize) {
        if (opened == false) { return; }
        size = nsize;
    }
    auto get_size() const {
        if (opened) { return size; }
        return glm::vec2{100.f, 20.f};
    }
    auto is_opened() const { return opened; }
    auto toggle_open() { return opened = !opened; }
    const auto &get_storage() const { return storage; }
    auto &get_storage() { return storage; }

    NodeID id;
    NodeType type{NodeType::None};

  private:
    glm::vec2 position{0.f};
    glm::vec2 min_size{0.f}, max_size{0.f};
    glm::vec2 size{150.f, 50.f};

    bool opened{true};
    std::unordered_map<std::string, std::any> storage;

    inline static uint32_t gid = 1u;
};

class RenderGraphGUI {
    struct Colors {
        inline static glm::u8vec4 grid{200, 200, 200, 40};
        inline static glm::u8vec4 node_clicked{255, 255, 255, 255};
        inline static glm::u8vec4 node_hovered{150, 150, 150, 255};
        inline static glm::u8vec4 node{50, 50, 50, 255};
        inline static glm::u8vec4 node_outline{255};
        inline static glm::u8vec4 window{30, 30, 30, 255};

        inline static glm::u8vec4 menubar_vao{227, 177, 39, 255};
        inline static glm::u8vec4 menubar_stage{86, 56, 150, 255};

        inline static glm::u8vec4 node_connection_line{113, 204, 225, 255};
    };

  public:
    RenderGraphGUI();

    void draw();
    bool is_open() const { return open; }
    void open_widget() { open = true; }

    Node *get_node(NodeID);

  private:
    void add_node(NodeType type);
    void mouse_node_interactions();
    void move_node_to_front(NodeID id);

    inline void enable_node_interaction() { allow_node_interaction = true; }
    inline void disable_node_interaction() { allow_node_interaction = false; }

    void draw_background();
    void draw_buffer_list();
    void draw_canvas();
    void draw_nodes();
    void draw_node_contents(Node *);
    void draw_connection_button(Node *node, const char *name, bool is_output);

    std::vector<Node> nodes{};
    eng::SortedVectorUnique<NodeIO> ios;
    std::unordered_map<uint32_t, std::string> buffers_names;
    int editing_buffer_name;
    char *new_buffer_name{nullptr};

    glm::vec2 window_start{0.f}, window_size{0.f};
    glm::vec2 canvas_start{0.f}, canvas_size{0.f};
    glm::vec2 pan_offset{0.f};
    float grid_step  = 64.f;
    float zoom_speed = 100.f;
    float scale{1.f};
    const float scale_min{0.01f}, scale_max{3.f};

    ImDrawList *bgdraw_list{nullptr};
    bool open{false};
    bool canvas_panning{false};
    bool resource_dragging{false};
    bool allow_node_interaction{true};

    enum NodeAction_ {
        NodeAction_None,
        NodeAction_Hovering,
        NodeAction_Down,
        NodeAction_Dragging,
        NodeAction_Resizing
    };
    NodeAction_ active_node_action = NodeAction_None;
    NodeID active_node_id{0u};

    bool line_dragging{false};

    struct {
        NodeID from{0}, to{0};
        glm::vec2 line_dragging_start{0.f}, line_dragging_end{0.f};
    } node_connection;
    std::string line_dragging_name;
};