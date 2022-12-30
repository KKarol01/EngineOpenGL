#pragma once

#include <vector>
#include <string>
#include <memory>
#include <any>
#include <unordered_map>

#include <glm/glm.hpp>

struct ImDrawList;

struct NodeDataStorage {
    std::unordered_map<std::string, std::any> storage;
};

enum class NodeType { DepthTest, VAO };

struct Node {
  private:
    Node() = default;

  public:
    auto start() const { return position; }
    auto end() const { return position + size; }

    NodeType type{0u};
    glm::vec2 position{0.f};
    glm::vec2 min_size, max_size;
    glm::vec2 size{150.f, 50.f};
    
    glm::vec2 drag_start{0.f}, drag_end{0.f};
    bool mouse_down{false}, mouse_hovering{false}, mouse_dragging{false};
    uint32_t id = gid++;
    NodeDataStorage storage;

    inline static float corner_rounding = 5.f;
    inline static float border_size     = .7f;
    inline static float padding         = 2.f;
    inline static uint32_t gid          = 0u;

    friend class NodeBuilder;
};

struct NodeBuilder {

    Node build(NodeType type);
};

class RenderGraphGUI {
    struct Colors {
        inline static glm::u8vec4 grid{200, 200, 200, 40};
        inline static glm::u8vec4 node_clicked{255, 255, 255, 255};
        inline static glm::u8vec4 node_hovered{150, 150, 150, 255};
        inline static glm::u8vec4 node{50, 50, 50, 255};
        inline static glm::u8vec4 node_outline{255};
        inline static glm::u8vec4 window{30, 30, 30, 255};
    };

  public:
    RenderGraphGUI();

    void draw();

    bool is_open() const { return open; }
    void open_widget() { open = true; }

  private:
    void handle_panning();
    void handle_node_mouse_interaction();
    void draw_grid();
    void draw_nodes();
    void draw_node_contents(Node *);
    void draw_connection_dot(Node *, bool left, bool center);
    void draw_resource_list();
    void draw_canvas();

    bool mouse_over_node(const Node &n);

    std::vector<Node> nodes{};
    std::unordered_map<uint32_t, std::string> buffer_names;

    glm::vec2 canvas_size{0.f};
    glm::vec2 canvas_start{0.f}, canvas_end{0.f};
    glm::vec2 pan_offset{0.f};
    float grid_step = 64.f;
    float zoom_speed = 5.f;
    ImDrawList *draw_list{nullptr};
    NodeBuilder node_builder;
    bool open{true};
    Node *inode{nullptr};
    bool canvas_panning{false};
    bool resource_dragging{false};
    bool can_move_nodes{true};
};