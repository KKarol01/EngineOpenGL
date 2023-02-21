#pragma once

#include <vector>
#include <string>
#include <memory>
#include <any>
#include <unordered_map>
#include <functional>

#include <glm/glm.hpp>

struct ImDrawList;
typedef uint32_t NodeID;
enum class NodeType { DepthTest, VAO, Stage };

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

    NodeID id;
    NodeType type{0u};
    glm::vec2 position{0.f};
    glm::vec2 min_size{0.f}, max_size{0.f};
    glm::vec2 size{150.f, 50.f};

    glm::vec2 drag_start{0.f}, drag_end{0.f};
    bool clicked{false}, hovered{false}, down{false};
    bool resizing{false}, moving{false};
    bool dragging_new_link{false};
    bool opened{true};
    std::string hovered_dot;
    std::unordered_map<std::string, std::any> storage;

    inline static float corner_rounding = 5.f;
    inline static float border_size     = .7f;
    inline static float padding         = 2.f;
    inline static uint32_t gid          = 1u;
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
    };

  public:
    RenderGraphGUI();

    void draw();
    bool is_open() const { return open; }
    void open_widget() { open = true; }

    Node *get_node(NodeID);

  private:
    void draw_nodes();
    void draw_node_contents(Node *);
    void draw_resource_list();
    void add_node(NodeType type);
    void mouse_node_interactions();
    void draw_buffer_list();
    void draw_canvas();
    void move_node_to_front(NodeID id);

    int inode = -1;
    std::vector<Node> nodes{};
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
    bool open{true};
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
    uint32_t line_dragging_node_of_origin;
    glm::vec2 line_dragging_start;
    std::string line_dragging_name;
};