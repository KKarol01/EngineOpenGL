#pragma once

#include "pipeline.hpp"
#include "typedefs.hpp"
#include "../types/types.hpp"

template <typename T, typename COMP_LESS> class REAllocator {
    struct StorageWrapper {

        bool operator<(const StorageWrapper &other) const noexcept { return id < other.id; }
        bool operator==(const StorageWrapper &other) const noexcept { return id == other.id; }

        uint32_t id;
        T value;
    };

  public:
    template <typename... ARGS> auto allocate(ARGS &&...args) {
        auto &e = storage.emplace(gid, T{std::forward<ARGS>(args)...});
        return gid++;
    }
    T &get_resource(uint32_t id) {
        return storage.find(id, [](auto &&e, auto &&v) { return e.id < v; })->value;
    }
    const T &get_resource(uint32_t id) const { return get_resource(id); }

  private:
    void iterate(std::function<void(T &)> f) {
        for (auto &v : storage) { f(v.value); }
    }

    uint32_t gid{0u};
    eng::SortedVectorUnique<StorageWrapper> storage;

    friend class RE;
};

class RE {

  public:
    void render() {
        pipelines.iterate([](auto &p) { p.render(); });
    }
    void allocate_model(PipelineID pid, const Model &m) { pipelines.get_resource(pid).allocate_model(m); }
    void instance_model(uint32_t model_id, PipelineID pid) { get_pipeline(pid).create_instance(model_id); }

    template <typename... ARGS> auto create_pipeline(ARGS &&...args) {
        return pipelines.allocate(std::forward<ARGS>(args)...);
    }
    template <typename... ARGS> auto create_buffer(ARGS &&...args) {
        return buffers.allocate(std::forward<ARGS>(args)...);
    }
    template <typename... ARGS> auto create_vao(ARGS &&...args) { return vaos.allocate(std::forward<ARGS>(args)...); }

    GLBuffer &get_buffer(BufferID bid) { return buffers.get_resource(bid); }
    GLVao &get_vao(VaoID vid) { return vaos.get_resource(vid); }
    RenderingPipeline &get_pipeline(PipelineID vid) { return pipelines.get_resource(vid); }
    const GLBuffer &get_buffer(BufferID bid) const { return buffers.get_resource(bid); }
    const GLVao &get_vao(VaoID vid) const { return vaos.get_resource(vid); }
    const RenderingPipeline &get_pipeline(PipelineID vid) const { return pipelines.get_resource(vid); }

  private:
    uint32_t pipeline_id{0u};

    using ALLOC_COMP = decltype([](auto &&a, auto &&b) { return a.handle < b.handle; });
    REAllocator<GLVao, ALLOC_COMP> vaos;
    REAllocator<GLBuffer, ALLOC_COMP> buffers;
    REAllocator<RenderingPipeline, ALLOC_COMP> pipelines;
};