#pragma once

#include <vector>
#include <initializer_list>
#include <cstdint>
#include <functional>
#include <map>

struct BUFFEROBJECT {
    uint32_t handle{0u};
    uint32_t id{0u}, offset{0u}, stride{0u};

    BUFFEROBJECT() = default;
    template <typename T>
    BUFFEROBJECT(uint32_t id, std::vector<T> &data, uint32_t flags = 0u)
        : BUFFEROBJECT(id, data.data(), data.size(), sizeof(T), 0u, flags) {}
    template <typename T>
    BUFFEROBJECT(uint32_t id, std::vector<T> &data, uint32_t stride, uint32_t offset, uint32_t flags = 0u)
        : BUFFEROBJECT(id, data.data(), data.size(), stride, offset, flags) {}

    BUFFEROBJECT(uint32_t id, void *data, uint32_t arr_length, uint32_t stride, uint32_t offset, uint32_t flags = 0u);
    BUFFEROBJECT(BUFFEROBJECT &&) noexcept;
    BUFFEROBJECT &operator=(BUFFEROBJECT &&) noexcept;
};

struct ATTRFORMATCOMMON {
    uint32_t id, buffer, normalized;
    enum class GL_FUNC { FLOAT, INT, LONG } format_func = GL_FUNC::FLOAT;

    ATTRFORMATCOMMON(uint32_t id, uint32_t buffer, GL_FUNC format_func = GL_FUNC::FLOAT, uint32_t normalized = 0u)
        : id{id}, buffer{buffer}, normalized{normalized}, format_func{format_func} {}
};
template <typename BEHAVIOR> struct ATTRFORMAT : ATTRFORMATCOMMON {};
struct BEHAVIORSAMEFORMAT;
struct BEHAVIORSAMETYPE;
struct BEHAVIORCUSTOMFORMAT;
using ATTRSAMEFORMAT    = ATTRFORMAT<BEHAVIORSAMEFORMAT>;
using ATTRSAMETYPE      = ATTRFORMAT<BEHAVIORSAMETYPE>;
using ATTRCUSTORMFORMAT = ATTRFORMAT<BEHAVIORCUSTOMFORMAT>;

template <> struct ATTRFORMAT<BEHAVIORSAMEFORMAT> : ATTRFORMATCOMMON {
    ATTRFORMAT(uint32_t id, uint32_t buffer, GL_FUNC format_func = GL_FUNC::FLOAT, uint32_t normalized = 0u)
        : ATTRFORMATCOMMON{id, buffer, format_func, normalized} {}
};
template <> struct ATTRFORMAT<BEHAVIORCUSTOMFORMAT> : ATTRFORMATCOMMON {
    uint32_t size, type, type_size, offset;

    ATTRFORMAT(uint32_t id,
               uint32_t buffer,
               uint32_t size,
               uint32_t type,
               uint32_t type_size,
               uint32_t offset,
               GL_FUNC format_func = GL_FUNC::FLOAT,
               uint32_t normalized = 0u)
        : ATTRFORMATCOMMON{id, buffer, format_func, normalized}, size{size}, type{type}, type_size{type_size},
          offset{offset} {}
};
template <> struct ATTRFORMAT<BEHAVIORSAMETYPE> : ATTRFORMATCOMMON {
    uint32_t size;

    ATTRFORMAT(
        uint32_t id, uint32_t buffer, uint32_t size, GL_FUNC format_func = GL_FUNC::FLOAT, uint32_t normalized = 0u)
        : ATTRFORMATCOMMON{id, buffer, format_func, normalized}, size{size} {}
};

struct VAO {
    uint32_t handle;
    uint32_t indice_count;
    BUFFEROBJECT ebo;
    std::map<uint32_t, BUFFEROBJECT> buffers;
    std::function<void()> draw;

    VAO();
    VAO(VAO &&) noexcept;
    VAO &operator=(VAO &&) noexcept;
    ~VAO();

    void bind() const;

    void insert_vbo(BUFFEROBJECT &&buff);
    void configure_binding(uint32_t id, uint32_t buffer_id, uint32_t stride, uint32_t offset);
    void insert_ebo(uint32_t indice_count, BUFFEROBJECT &&buff);

    void configure(std::initializer_list<ATTRCUSTORMFORMAT> formats);
    void configure(uint32_t size, uint32_t type, uint32_t type_size, std::initializer_list<ATTRSAMEFORMAT> formats);
    void configure(uint32_t type, uint32_t type_size, std::initializer_list<ATTRSAMETYPE> formats);

    template <typename F, typename... DRAW_ARGS> void set_draw_func(F &&f, DRAW_ARGS &&...draw_args) {
        draw = std::bind(f, std::forward<DRAW_ARGS>(draw_args)...);
    }

  private:
    void _configure_impl(ATTRFORMATCOMMON::GL_FUNC func,
                         uint32_t idx,
                         uint32_t buffer,
                         uint32_t size,
                         uint32_t type,
                         uint32_t normalized,
                         uint32_t offset);
};
