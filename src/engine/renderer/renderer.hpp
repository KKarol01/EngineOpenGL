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
        uint32_t id{TypeIdGen<Resource>::unique()};
    };

    enum class RenderPass {
        Forward,
    };

    struct Material : public IdResource<Material> {
        std::unordered_map<RenderPass, ShaderProgram *> passes;
    };

    struct Mesh : public IdResource<Mesh> {
        Material *material{nullptr};

        glm::mat4 transform{1.f};

        std::vector<float> vertices;
        std::vector<unsigned> indices;
    };

    struct Object : public IdResource<Object> {
        std::vector<Mesh> meshes;
    };

    struct RenderObject : public IdResource<RenderObject> {
        uint32_t object_id;
        Handle<Mesh> mesh;
        Handle<Material> material;
        glm::mat4 transform;
    };

    struct PassMaterial {
        ShaderProgram *prog;
    };

    struct PassObject : IdResource<PassObject> {
        PassMaterial mat;
        Handle<Mesh> mesh;
        Handle<RenderObject> render_object;
    };

    struct FlatBatch {
        uint32_t batch_id;
        Handle<PassObject> object;
    };

    struct IndirectBatch {
        Handle<Mesh> mesh;
        PassMaterial material;
        uint32_t first, count;
    };

    struct MultiBatch {
        uint32_t first, count;
    };

    class Renderer;
    class MeshPass {
      public:
        void refresh(Renderer *r);

      private:
        uint32_t get_batch_id(Handle<Mesh>, Handle<Material>);
        PassObject &get_pass_object(Handle<PassObject> p) {
            return *std::find_if(pass_objects.begin(), pass_objects.end(), [id = p.id](auto &&e) {
                return e.id == id;
            });
        }

      public:
        std::vector<Handle<RenderObject>> unbatched;
        std::vector<PassObject> pass_objects;

        std::vector<MultiBatch> multi_batches;
        std::vector<IndirectBatch> indirect_batches;
        std::vector<FlatBatch> flat_batches;

      private:
        std::vector<std::pair<Handle<Mesh>, Handle<Material>>> _batch_ids;
    };

    struct DrawElementsIndirectCommand {
        uint32_t count{0u};
        uint32_t instance_count{0u};
        uint32_t first_index{0u};
        uint32_t base_vertex{0u};
        uint32_t base_instance{0u};
    };

    class Renderer {
      public:
        Renderer();

        void register_object(const Object *o);
        void render();

        RenderObject &get_render_object(Handle<RenderObject> h);
        Material &get_material(Handle<Material> h);

      private:
        template <typename Resource> Handle<Resource> get_resource_handle(const Resource &m);

        template <typename Iter>
        typename Iter::value_type *try_find_idresource(uint32_t id, Iter &it);

        template <typename Iter>
        typename Iter::value_type *try_find_idresource_binary(uint32_t id, Iter &it);

        MeshPass forward_pass;

        std::vector<Handle<RenderObject>> _dirty_objects;

        std::unordered_map<uint32_t, uint32_t> _mesh_instance_count;
        std::vector<RenderObject> _renderables;
        std::vector<Mesh> _meshes;
        std::vector<Material> _materials;

        uint32_t vao;
        GLBuffer commands_buffer{GL_DYNAMIC_STORAGE_BIT};
        GLBuffer geometry_buffer{GL_DYNAMIC_STORAGE_BIT};
        GLBuffer index_buffer{GL_DYNAMIC_STORAGE_BIT};
        GLBuffer mesh_data_buffer{GL_DYNAMIC_STORAGE_BIT};
        // GLBuffer {GL_DYNAMIC_STORAGE_BIT};
    };

} // namespace eng
