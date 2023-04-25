#pragma once

#include <cstdint>
#include <vector>
#include <concepts>
#include <unordered_map>
#include <algorithm>
#include <compare>
#include <utility>

#include <engine/gpu/shaderprogram/shader.hpp>
#include <engine/gpu/buffers/buffer.hpp>
#include <engine/types/idallocator.hpp>
#include <engine/renderer/typedefs.hpp>
#include <glm/glm.hpp>

namespace eng {
    class ShaderProgram;
    class GLBuffer;
    class GLVao;
} // namespace eng

namespace eng {
    template <typename T> struct HandleCounter {
        static inline uint32_t request_handle() { return gid++; }

        static inline uint32_t gid{1u};
    };
    template <typename T> struct Handle {
        explicit Handle() = default;
        explicit Handle(uint32_t h) : handle{h} {}
        explicit operator uint32_t() { return handle; }
        auto operator<=>(const Handle<T> &) const = default;

        static inline Handle<T> get() { return Handle<T>(HandleCounter<T>::request_handle()); }

        uint32_t handle{0u};
    };

    template <typename T> class HandleVec {
      public:
        Handle<T> push_back(const T &t) { return _storage.emplace_back(Handle<T>::get(), T{t}).first; }
        T *try_find(Handle<T> handle) {
            auto it = std::lower_bound(
                _storage.begin(), _storage.end(), handle, [](auto &&e, auto &&v) { return e.first.handle < v.handle; });

            if (it == _storage.end() || it->first.handle != handle.handle) { return nullptr; }

            return &it->second;
        }

        decltype(auto) begin() { return _storage.begin(); }
        decltype(auto) end() { return _storage.end(); }
        decltype(auto) operator[](size_t i) { return _storage[i].second; }
        decltype(auto) operator[](Handle<T> handle) { return *try_find(handle); }

      private:
        std::vector<std::pair<Handle<T>, T>> _storage;
    };

    enum class PipelinePass { Forward };

    struct MaterialPass {
        std::unordered_map<PipelinePass, ShaderProgram *> pipelines;
    };

    struct ShaderData {
        std::vector<float> data;
        uint32_t bytes_per_instance{0u};
    };

    struct Material {
        MaterialPass *pass{nullptr};
        ShaderData *data{nullptr};
    };

    struct Mesh {
        uint32_t id;
        Material *material{nullptr};
        bool use_forward_pass{true};
        glm::mat4 transform{1.f};
        std::vector<float> vertices;
        std::vector<unsigned> indices;
    };

    struct RenderObject;

    struct PassMaterial {
        ShaderProgram *program;
        ShaderData *data;
    };
    struct PassObject {
        PassMaterial material;
        Handle<Mesh> mesh;
        Handle<RenderObject> original;
    };

    struct RenderBatch {
        Handle<PassObject> object;
        uint32_t id;
    };
    struct IndirectBatch {
        Handle<Mesh> mesh;
        PassMaterial material;
        uint32_t first, count;
    };
    struct MultiBatch {
        uint32_t first, count;
    };

    struct Renderer;

    struct MeshPass {

        void refresh(Renderer *r);

        HandleVec<PassObject> objects;
        std::vector<MultiBatch> multibatches;
        std::vector<IndirectBatch> indirectbatches;
        std::vector<RenderBatch> flatbatches;
        std::vector<Handle<RenderObject>> unbatched;

        bool needs_refresh{false};

      private:
        uint32_t assign_batch_id(const RenderObject &ro);

        using BatchId = uint32_t;
        std::map<std::pair<Handle<Mesh>, Handle<Material>>, BatchId> batch_ids;
    };

    struct Object {
        uint32_t id;
        std::vector<Mesh> meshes;
    };

    struct RenderObject {
        Handle<Material> material;
        Handle<Mesh> mesh;
        glm::mat4 transform;
    };

    class Renderer {
      public:
        Renderer();

        Handle<RenderObject> register_object(const Object *object);

        ShaderProgram *empty_program() {
            return programs.emplace_back(Handle<ShaderProgram>::get(), std::make_unique<ShaderProgram>()).second.get();
        }
        MaterialPass *empty_material_pass() {
            return material_passes.emplace_back(Handle<MaterialPass>::get(), std::make_unique<MaterialPass>())
                .second.get();
        }
        Material *get_material(Handle<Material> handle) { return &find_resource_by_handle(handle, materials).second; }
        MaterialPass *get_material_pass(Handle<MaterialPass> handle) {
            return find_resource_by_handle(handle, material_passes).second.get();
        }
        RenderObject *get_render_object(Handle<RenderObject> handle) {
            return &find_resource_by_handle(handle, renderables).second;
        }

        void render();

      private:
        struct DrawElementsIndirectCommand {
            uint32_t count;
            uint32_t instanceCount;
            uint32_t firstIndex;
            int baseVertex;
            uint32_t baseInstance;
        };
        struct MeshBufferLocation {
            bool operator==(Handle<Mesh> handle) const { return mesh == handle; }

            Handle<Mesh> mesh;
            uint32_t first_index, index_count;
            int first_vertex;
            uint32_t vertex_count;
        };

        template <typename T, typename Iterable> decltype(auto) find_resource_by_handle(Handle<T> h, Iterable &it) {
            return *std::lower_bound(
                it.begin(), it.end(), h, [](auto &&e, auto &&v) { return e.first.handle < v.handle; });
        }

        Handle<Material> get_material_handle(const Material *mat);
        Handle<Mesh> get_mesh_handle(const Mesh *m);

        MeshPass forward_pass;

        template <typename T> using vec_pair_handle_type  = std::vector<std::pair<Handle<T>, T>>;
        template <typename T> using vec_pair_handle_ptype = std::vector<std::pair<Handle<T>, std::unique_ptr<T>>>;
        vec_pair_handle_ptype<ShaderProgram> programs;
        vec_pair_handle_ptype<MaterialPass> material_passes;

        HandleVec<Mesh> meshes;
        std::vector<MeshBufferLocation> mesh_buffer_locations;
        vec_pair_handle_type<Material> materials;

        std::vector<Handle<Mesh>> nongpu_resident_meshes;

        vec_pair_handle_type<RenderObject> renderables;
        std::vector<Handle<RenderObject>> newly_added_objects;
        std::vector<DrawElementsIndirectCommand> draw_commands;

        GLBuffer geometry_buffer{GL_DYNAMIC_STORAGE_BIT}, index_buffer{GL_DYNAMIC_STORAGE_BIT};
        GLBuffer draw_buffer{GL_DYNAMIC_STORAGE_BIT};
        GLBuffer ssbo{GL_DYNAMIC_STORAGE_BIT}, ssbo_attributes{GL_DYNAMIC_STORAGE_BIT};

        GLVaoID vao;
    };

} // namespace eng
