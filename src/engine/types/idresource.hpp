#pragma once

#include <cstdint>
#include <compare>

namespace eng {
    template <typename Resource> struct Handle {
        explicit Handle() = default;
        explicit Handle(uint32_t id) : id{id} {}
        operator uint32_t() const { return id; }
        auto operator<=>(const Handle<Resource> &) const = default;

        static inline Handle<Resource> get() {
            static uint32_t gid{0u};
            return Handle<Resource>{++gid};
        }

        uint32_t id;
    };
    template <typename Type> struct TypeIdGen {
        static inline uint32_t unique() {
            static uint32_t gid{0u};
            return ++gid;
        }
    };
    template <typename Resource> struct IdResource {
        auto operator<=>(const IdResource<Resource> &) const = default;
        auto operator<=>(Handle<Resource> h) const { return id <=> h.id; }
        auto operator==(Handle<Resource> h) const { return id == h.id; }

        uint32_t id{TypeIdGen<Resource>::unique()};
    };
} // namespace eng
