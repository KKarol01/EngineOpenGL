#pragma once

#include <vector>
#include <initializer_list>
#include <cstdint>
#include <functional>
#include <map>

#include "../../signal/signal.hpp"

struct GLBufferDescriptor {
    bool operator==(const GLBufferDescriptor &other) const noexcept { return handle == other.handle; }

    std::uint32_t handle{0u}, flags{0u};
    std::size_t size{0u}, capacity{0u}, stride{0u};

    Signal<uint32_t> on_handle_change;
};

struct GLBuffer {
    explicit GLBuffer(uint32_t flags);
    GLBuffer(GLBuffer &&) noexcept;
    GLBuffer &operator=(GLBuffer &&) noexcept;
    GLBuffer(void *data, uint32_t size_bytes, uint32_t flags);
    template <typename T>
    GLBuffer(std::vector<T> &data, uint32_t flags = 0u) : GLBuffer(data.data(), data.size() * sizeof(T), flags) {}

    ~GLBuffer();

    void push_data(void *data, size_t size_bytes);

    GLBufferDescriptor descriptor;

  private:
    void resize(size_t required_size);

    const constexpr static inline float GROWTH_FACTOR{1.61f};
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

    std::uint32_t size, type, type_size, offset;
};
template <> struct ATTRFORMAT<BEHAVIORSAMETYPE> : ATTRFORMATCOMMON {
    ATTRFORMAT(
        uint32_t id, uint32_t buffer, uint32_t size, GL_FUNC format_func = GL_FUNC::FLOAT, uint32_t normalized = 0u)
        : ATTRFORMATCOMMON{id, buffer, format_func, normalized}, size{size} {}

    std::uint32_t size;
};

struct GLVao {

    GLVao();
    GLVao(GLVao &&) noexcept;
    GLVao &operator=(GLVao &&) noexcept;
    bool operator==(const GLVao &other) const noexcept { return handle == other.handle; }
    ~GLVao();

    void bind() const;

    void configure_binding(uint32_t id, uint32_t handle, size_t stride, size_t offset = 0u);
    void configure_ebo(uint32_t handle);

    void configure_attr(std::initializer_list<ATTRCUSTORMFORMAT> formats);
    void configure_attr(uint32_t vertices_size_bytes,
                        uint32_t type,
                        uint32_t type_size,
                        std::initializer_list<ATTRSAMEFORMAT> formats);
    void configure_attr(uint32_t type, uint32_t type_size, std::initializer_list<ATTRSAMETYPE> formats);

    uint32_t handle;

  private:
    void _configure_impl(ATTRFORMATCOMMON::GL_FUNC func,
                         uint32_t idx,
                         uint32_t buffer,
                         uint32_t vertices_size_bytes,
                         uint32_t type,
                         uint32_t normalized,
                         uint32_t offset);
};
