#include "ubo.hpp"

#include <glad/glad.h>
#include <glm/glm.hpp>
#include "../../engine.hpp"
#include "../../renderer/renderer.hpp"

eng::UBO::UBO(InitList init) {
    fill_entries(init, 1);
    create_storage(total_size, STORAGE_FLAGS);

    for (auto &[name, type] : init) {
        const void *dst{nullptr};
        std::visit([&dst](auto &e) { dst = static_cast<const void *>(&e); }, type);
        assert(dst);
        memcpy(get(name), dst, entries.at(name).size);
    }
}

eng::UBO::UBO(InitList init, const std::vector<const void *> &data) {
    fill_entries(init, data.size());

    // recalculation because of std140 struct alignment rules
    const auto round_to_multiple = [](size_t num, size_t mult) { return ((num + mult - 1ull) / mult) * mult; };

    auto max_size = 0ull;
    for (const auto &d : init) max_size = glm::max((size_t)max_size, get_data_size(d.second));
    stride     = round_to_multiple(total_size, max_size);
    total_size = stride * data.size();

    create_storage(total_size, STORAGE_FLAGS);

    assert(mapped);
    assert(num_elems == data.size());
    for (auto i = 0u; i < num_elems; ++i) {
        assert(i * stride < total_size);
        memcpy(static_cast<char *>(ptr) + i * stride, data[i], stride);
    }
}

void eng::UBO::bind(uint32_t binding_idx) const {
    glBindBufferBase(GL_UNIFORM_BUFFER,
                     binding_idx,
                     eng::Engine::instance().renderer_->get_buffer(storage_buffer_id).descriptor.handle);
}

void eng::UBO::create_storage(size_t size, uint32_t flags) {
    storage_buffer_id = eng::Engine::instance().renderer_->create_buffer(flags);

    auto &buff = eng::Engine::instance().renderer_->get_buffer(storage_buffer_id);
    buff.push_data(nullptr, size);
    map_buffer(GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
}

eng::GLBuffer &eng::UBO::get_storage() { return eng::Engine::instance().renderer_->get_buffer(storage_buffer_id); }

void eng::UBO::map_buffer(uint32_t flags) {
    ptr = glMapNamedBufferRange(get_storage().descriptor.handle, 0, total_size, flags);
    assert(ptr);
    mapped = true;
}

void eng::UBO::unmap_buffer() {
    glUnmapNamedBuffer(get_storage().descriptor.handle);
    ptr    = nullptr;
    mapped = false;
}

void eng::UBO::fill_entries(const InitList &list, size_t num_entries) {
    for (auto &[name, type] : list) {
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

    stride    = total_size;
    num_elems = num_entries;
}

size_t eng::UBO::get_data_size(const Types &t) {
    return std::visit([](auto &&t) { return sizeof(t); }, t);
}

const uint32_t eng::UBO::STORAGE_FLAGS
    = GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;