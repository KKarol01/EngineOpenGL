#pragma once

#include <variant>
#include <glm/glm.hpp>
#include <initializer_list>
#include <unordered_map>
#include <cstdint>
#include <cassert>
#include "../../renderer/typedefs.hpp"

class UBO {

    using TS = std::variant<float, int, glm::vec4, glm::mat4>;
    struct BufferEntry {
        size_t offset{0};
        uint32_t size{0};
    };

  public:
    UBO(std::initializer_list<std::pair<std::string, TS>> init);

    template <typename T = void> T *get(std::string_view name) {
        assert(mapped);
        return static_cast<T *>(static_cast<void *>(static_cast<char *>(ptr) + entries.at(name.data()).offset));
    }
    template<typename T> void set(std::string_view name, T&& data) {
        assert(mapped);
        memcpy(static_cast<void *>(static_cast<char *>(ptr) + entries.at(name.data()).offset),
               &data,
               entries.at(name.data()).size);
    }

    void bind(uint32_t binding_idx) const;

  private:
    bool mapped{false};
    void *ptr{nullptr};
    BufferID storage;
    size_t total_size{0};
    std::unordered_map<std::string, BufferEntry> entries;
};