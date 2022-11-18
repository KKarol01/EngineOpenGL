#pragma once

#include <vector>
#include <initializer_list>
#include <cstdint>
#include <functional>
#include <map>
#include <variant>

#include "../../signal/signal.hpp"
#include "../../renderer/typedefs.hpp"

struct GLBufferDescriptor {

    explicit GLBufferDescriptor() = default;
    GLBufferDescriptor(uint32_t flags) : flags{flags} {}
    bool operator==(const GLBufferDescriptor &other) const noexcept { return handle == other.handle; }

    std::uint32_t handle{0u}, flags{0u};
    std::size_t size{0u}, capacity{0u};

    Signal<uint32_t> on_handle_change;
};

struct GLBuffer {
    GLBuffer() = default;
    explicit GLBuffer(GLBufferDescriptor desc);
    GLBuffer(GLBuffer &&) noexcept;
    GLBuffer &operator=(GLBuffer &&) noexcept;
    GLBuffer(void *data, uint32_t size_bytes, GLBufferDescriptor desc);
    template <typename T>
    GLBuffer(std::vector<T> &data, GLBufferDescriptor desc) : GLBuffer(data.data(), data.size() * sizeof(T), desc) {}
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

struct GLVaoBinding {
    GLVaoBinding(uint32_t binding, BufferID buffer, size_t stride, size_t offset = 0)
        : binding{binding}, buffer{buffer}, stride{stride}, offset{offset} {}

    uint32_t binding, buffer;
    size_t stride, offset{0};
};

struct GLVaoAttrCustom {
    GLVaoAttrCustom(std::initializer_list<ATTRCUSTORMFORMAT> formats) : formats{formats} {}
    std::vector<ATTRCUSTORMFORMAT> formats;
};
struct GLVaoAttrSameFormat {
    GLVaoAttrSameFormat(uint32_t size,
                        uint32_t type,
                        uint32_t type_size_bytes,
                        std::initializer_list<ATTRSAMEFORMAT> formats)
        : size{size}, type{type}, type_size_bytes{type_size_bytes}, formats{formats} {}
    uint32_t size, type, type_size_bytes;
    std::vector<ATTRSAMEFORMAT> formats;
};
struct GLVaoAttrSameType {
    GLVaoAttrSameType(uint32_t type, uint32_t type_size_bytes, std::initializer_list<ATTRSAMETYPE> formats)
        : type{type}, type_size_bytes{type_size_bytes}, formats{formats} {}

    uint32_t type, type_size_bytes;
    std::vector<ATTRSAMETYPE> formats;
};

struct GLVaoDescriptor {
    template <typename T> using vec_t = std::vector<T>;
    using format_variants             = std::variant<GLVaoAttrCustom, GLVaoAttrSameFormat, GLVaoAttrSameType>;

    GLVaoDescriptor(std::initializer_list<GLVaoBinding> vbo_bindings, format_variants formats)
        : vbo_bindings{vbo_bindings}, formats{formats} {}
    GLVaoDescriptor(std::initializer_list<GLVaoBinding> vbo_bindings, format_variants formats, BufferID ebo_buffer)
        : vbo_bindings{vbo_bindings}, formats{formats}, ebo_buffer{ebo_buffer}, ebo_set{true} {}

    vec_t<GLVaoBinding> vbo_bindings;
    format_variants formats;
    BufferID ebo_buffer{0};
    bool ebo_set = false;
};

struct GLVao {

    GLVao() = default;
    explicit GLVao(GLVaoDescriptor desc);
    GLVao(GLVao &&) noexcept;
    GLVao &operator=(GLVao &&) noexcept;
    bool operator==(const GLVao &other) const noexcept { return handle == other.handle; }
    ~GLVao();

    void bind() const;

  private:
    void configure_binding(uint32_t id, uint32_t handle, size_t stride, size_t offset = 0u);
    void configure_ebo(uint32_t handle);

    void configure_attr(std::vector<ATTRCUSTORMFORMAT> formats);
    void configure_attr(uint32_t size, uint32_t type, uint32_t type_size, std::vector<ATTRSAMEFORMAT> formats);
    void configure_attr(uint32_t type, uint32_t type_size, std::vector<ATTRSAMETYPE> formats);

    uint32_t handle;

    void _configure_impl(ATTRFORMATCOMMON::GL_FUNC func,
                         uint32_t idx,
                         uint32_t buffer,
                         uint32_t size,
                         uint32_t type,
                         uint32_t normalized,
                         uint32_t offset);
};
