#pragma once

#include "typedefs.hpp"
#include "../wrappers/buffer/buffer.hpp"
#include "../types/types.hpp"

template <typename T> class REAllocator {
    struct TypeWrapper {
        template <typename... ARGS> TypeWrapper(ARGS &&...args) : type{std::forward<ARGS>(args)...} {}

        TypeWrapper(TypeWrapper &&other) noexcept { *this = std::move(other); }
        TypeWrapper &operator=(TypeWrapper &&other) noexcept {
            id   = other.id;
            type = std::move(other.type);
            return *this;
        }

        inline bool operator<(const TypeWrapper &other) const noexcept { return id < other.id; }
        inline bool operator==(const TypeWrapper &other) const noexcept { return id == other.id; }

        uint32_t id{gid++};
        T type;

        static inline uint32_t gid{0u};
    };

    template <typename... ARGS> auto allocate(ARGS &&...args) {
        return storage.emplace(std::forward<ARGS>(args)...).id;
    }
    auto &get(uint32_t id) {
        return *storage.find(id, [](auto &&e, auto &&v) { return e.id < v; });
    }

    eng::SortedVectorUnique<TypeWrapper> storage;

    friend class RE;
};

class RE {
  public:
    BufferID create_buffer(GLBufferDescriptor desc);
    VaoID create_vao(GLVaoDescriptor desc);
    GLBuffer &get_buffer(BufferID id);
    GLVao &get_vao(VaoID id);

  private:
    REAllocator<GLBuffer> buffers;
    REAllocator<GLVao> vaos;
};