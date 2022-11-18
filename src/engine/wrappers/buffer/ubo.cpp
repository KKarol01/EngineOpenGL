#include "ubo.hpp"

#include <glad/glad.h>
#include "../../engine.hpp"
#include "../../renderer/renderer.hpp"

UBO::UBO(std::initializer_list<std::pair<std::string, TS>> init) {

    for (auto &[name, type] : init) {
        BufferEntry entry;

        std::visit(
            [this, &entry](auto &&t) {
                size_t carry = (sizeof(t) % 16 == 0) ? total_size % 16 : 0;
                entry        = {.offset = total_size + carry, .size = sizeof(t)};
                total_size += entry.size + carry;
            },
            type);

        entries[name] = entry;
    }

    auto &re = *eng::Engine::instance().renderer_;
    storage  = re.create_buffer(
        {GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT | GL_MAP_READ_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT});

    re.get_buffer(storage).push_data(nullptr, total_size);
    ptr    = glMapNamedBufferRange(re.get_buffer(storage).descriptor.handle,
                                0,
                                total_size,
                                GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_COHERENT_BIT | GL_MAP_PERSISTENT_BIT);
    mapped = true;

    for (auto &[name, type] : init) {
        const void *dst{nullptr};
        std::visit([&dst](auto &e) { dst = static_cast<const void *>(&e); }, type);
        assert(dst);
        memcpy(get(name), dst, entries.at(name).size);
    }
}

void UBO::bind(uint32_t binding_idx) const {
    glBindBufferBase(
        GL_UNIFORM_BUFFER, binding_idx, eng::Engine::instance().renderer_->get_buffer(storage).descriptor.handle);
}
