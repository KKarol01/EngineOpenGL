#pragma once

#include <vector>
#include <string>
#include <memory>
#include <unordered_map>

#include <glm/glm.hpp>

struct ImDrawList;

class NodeDataStorage {
    struct MetaData {
        MetaData(const std::string &name, size_t off) : name{name}, offset{off} {}

        std::string name;
        size_t offset{0u};
    };

  public:
    NodeDataStorage() = default;

    NodeDataStorage(NodeDataStorage &&n) noexcept { *this = std::move(n); }
    NodeDataStorage &operator=(NodeDataStorage &&n) noexcept {
        total_size = n.total_size;
        storage    = std::move(n.storage);
        data       = n.data;
        n.data     = nullptr;

        return *this;
    }
    ~NodeDataStorage() {
        if (data) delete[] data;
    }

    template <typename T> void register_data(const std::string &s, const T &t = {}) {
        storage.emplace_back(s, total_size);
        if (data == nullptr) data = (char *)malloc(sizeof(T));
        else
            data = static_cast<char *>(realloc(data, total_size + sizeof(T)));
        assert(data != nullptr);
        memcpy(data + total_size, &t, sizeof(T));
        total_size += sizeof(T);
    }
    template <typename T = void> T *get(const std::string &s) {
        auto it = std::find_if(storage.begin(), storage.end(), [&s](auto &&val) { return val.name == s; });
        assert(it != storage.end());
        const auto &md = *it;

        return static_cast<T *>(static_cast<void *>(data + md.offset));
    }

  private:
    size_t total_size{0u};
    std::vector<MetaData> storage;
    char *data = nullptr;
};

enum class NodeType { DepthTest };

struct Node {
  private:
    Node() = default;

  public:
    auto start() const { return position; }
    auto end() const { return position + size; }

    NodeType type{0u};
    glm::vec2 position{0.f};
    glm::vec2 size{150.f, 50.f};
    glm::vec2 drag_start{0.f}, drag_end{0.f};
    bool mouse_down{false}, mouse_hovering{false}, mouse_dragging{false};
    NodeDataStorage storage;

    inline static float corner_rounding = 5.f;
    inline static float border_size     = .7f;
    inline static float padding         = 2.f;

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
    void draw();

    bool is_open() const { return open; }
    void open_widget() { open = true; }

  private:
    void handle_panning();
    void handle_node_mouse_interaction();
    void draw_grid();
    void draw_nodes();
    void draw_node_contents(Node *);
    void draw_connection_dot(Node *);

    bool mouse_over_node(const Node &n);

    std::vector<Node> nodes{};

    glm::vec2 canvas_size{0.f};
    glm::vec2 canvas_start{0.f}, canvas_end{0.f};
    glm::vec2 pan_offset{0.f};
    float grid_step = 64.f;
    ImDrawList *draw_list{nullptr};
    NodeBuilder node_builder;
    bool open{true};
    Node *inode{nullptr};
    bool node_input_focus{false};
};