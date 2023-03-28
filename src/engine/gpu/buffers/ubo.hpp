#pragma once

#include <variant>
#include <vector>
#include <initializer_list>
#include <unordered_map>
#include <cstdint>
#include <cassert>

#include <glm/glm.hpp>

#include "buffer.hpp"
#include "../../renderer/renderer.hpp"

namespace eng {
    class UBO {
        using Types    = std::variant<float, int, glm::vec4, glm::mat4>;
        using InitList = std::initializer_list<std::pair<std::string, Types>>;
        struct BufferEntry {
            size_t offset{0};
            uint32_t size{0};
        };

      public:
        UBO(InitList init);
        UBO(InitList init, const std::vector<const void *> &data);

        template <typename T = void> T *get(std::string_view name, size_t idx = 0u) {
            assert(mapped);

            auto cptr                = static_cast<char *>(ptr);
            const auto data_start    = stride * idx;
            const auto member_offset = entries.at(name.data()).offset;

            auto vptr = static_cast<void *>(cptr + data_start + member_offset);
            return static_cast<T *>(vptr);
        }
        template <typename T> void set(std::string_view name, const T &data, size_t idx = 0u) {
            auto tptr              = get<T>(name, idx);
            const auto member_size = entries.at(name.data()).size;
            memcpy(tptr, static_cast<const void *>(&data), member_size);
        }

        void bind(uint32_t binding_idx) const;
        BufferID get_bufferid() const { return storage_buffer_id; }

      private:
        void create_storage(size_t size, uint32_t flags);
        GLBuffer &get_storage();
        void map_buffer(uint32_t flags);
        void unmap_buffer();
        void fill_entries(const InitList &list, size_t num_entries);
        size_t get_data_size(const Types &t);

      private:
        bool mapped{false};
        void *ptr{nullptr};
        BufferID storage_buffer_id;
        size_t total_size{0}, stride{0}, num_elems{0};
        std::unordered_map<std::string, BufferEntry> entries;

        static const uint32_t STORAGE_FLAGS;
    };
} // namespace eng