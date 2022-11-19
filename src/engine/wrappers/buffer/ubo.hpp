#pragma once

#include <variant>
#include <vector>
#include <glm/glm.hpp>
#include <initializer_list>
#include <unordered_map>
#include <cstdint>
#include <cassert>
#include "../../renderer/typedefs.hpp"
#include "../../wrappers/buffer/buffer.hpp"

class UBO {
    using TS        = std::variant<float, int, glm::vec4, glm::mat4>;
    using INIT_LIST = std::initializer_list<std::pair<std::string, TS>>;
    struct BufferEntry {
        size_t offset{0};
        uint32_t size{0};
    };

  public:
    UBO(INIT_LIST init);
    UBO(INIT_LIST init, const std::vector<const void *> &data);

    template <typename T = void> T *get(std::string_view name, size_t idx = 0u) {
        assert(mapped);

        auto cptr                = static_cast<char *>(ptr);
        const auto data_start    = stride * idx;
        const auto member_offset = entries.at(name.data()).offset;

        auto vptr = static_cast<void *>(cptr + data_start + member_offset);
        return static_cast<T *>(vptr);
    }
    template <typename T> void set(std::string_view name, const T &data, size_t idx = 0u) {
        auto tptr              = get(name, idx);
        const auto member_size = entries.at(name.data()).size;

        memcpy(tptr, static_cast<const void *>(&data), member_size);
    }

    void bind(uint32_t binding_idx) const;

  private:
    void create_storage(size_t size, uint32_t flags);
    GLBuffer &get_storage();
    void map_buffer(uint32_t flags);
    void unmap_buffer();
    void fill_entries(const INIT_LIST &list, size_t num_entries);
    size_t get_data_size(const TS &t);

  private:
    bool mapped{false};
    void *ptr{nullptr};
    BufferID storage;
    size_t total_size{0}, stride{0}, num_elems{0};
    std::unordered_map<std::string, BufferEntry> entries;

    static const uint32_t STORAGE_FLAGS;
};