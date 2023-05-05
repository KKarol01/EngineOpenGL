#pragma once

#include <cstdint>
#include <vector>
#include <concepts>
#include <unordered_map>
#include <algorithm>
#include <compare>
#include <utility>

#include <engine/gpu/shaderprogram/shader.hpp>
#include <engine/gpu/resource_manager/gpu_res_mgr.hpp>
#include <engine/gpu/buffers/buffer.hpp>
#include <engine/gpu/texture/texture.hpp>
#include <engine/types/idallocator.hpp>
#include <engine/types/idresource.hpp>
#include <glm/glm.hpp>

namespace eng {
    struct ShaderProgram;
    struct GLBuffer;
    struct GLVao;
} // namespace eng

namespace eng {

    enum class RenderPass {
        Forward,
    };

    struct Material : public IdResource<Material> {
        explicit Material() = default;
        explicit Material(std::unordered_map<TextureType, Texture *> textures,
                          std::unordered_map<RenderPass, ShaderProgram *> passes)
            : textures{textures}, passes{passes} {}
        Material(Material &&o) noexcept {
            id       = o.id;
            textures = std::move(o.textures);
            passes   = std::move(o.passes);
            o.id     = 0u;
        }

        std::unordered_map<TextureType, Texture *> textures;
        std::unordered_map<RenderPass, ShaderProgram *> passes;
    };

    struct Mesh : public IdResource<Mesh> {
        explicit Mesh() = default;
        explicit Mesh(std::vector<float> vertices,
                      std::vector<unsigned> indices,
                      Handle<Material> material,
                      glm::mat4 transform)
            : vertices{vertices}, indices{indices}, material{material}, transform{transform} {}

        std::vector<float> vertices;
        std::vector<unsigned> indices;
        Handle<Material> material;
        glm::mat4 transform{1.f};
    };

    struct Object : public IdResource<Object> {
        explicit Object() = default;
        explicit Object(const std::vector<Mesh> &meshes) : meshes{meshes} {}

        std::vector<Mesh> meshes;
    };

    struct RenderObject : public IdResource<RenderObject> {

        explicit RenderObject() = default;
        explicit RenderObject(uint32_t object_id,
                              Handle<Mesh> mesh,
                              Handle<Material> material,
                              const glm::mat4 &transform)
            : object_id{object_id}, mesh{mesh}, material{material}, transform{transform} {}

        uint32_t object_id;
        Handle<Mesh> mesh;
        Handle<Material> material;
        glm::mat4 transform;
    };

    struct PassMaterial {
        Handle<ShaderProgram> prog;
    };

    struct PassObject : IdResource<PassObject> {
        PassObject(PassMaterial mat, Handle<Mesh> mesh, Handle<RenderObject> ro) {
            this->mat           = mat;
            this->mesh          = mesh;
            this->render_object = ro;
        }
        PassMaterial mat;
        Handle<Mesh> mesh;
        Handle<RenderObject> render_object;
    };

    struct FlatBatch {
        FlatBatch(uint32_t bid, Handle<PassObject> h) : batch_id{bid}, object{h} {}
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

      private:
        MeshPass _forward_pass;

        std::vector<Handle<RenderObject>> _dirty_objects;
        std::unordered_map<uint32_t, uint32_t> _mesh_instance_count;

        GLVao *mesh_vao{nullptr};
        GLBuffer *commands_buffer{nullptr};
        GLBuffer *geometry_buffer{nullptr};
        GLBuffer *index_buffer{nullptr};
        GLBuffer *mesh_data_buffer{nullptr};
    };

} // namespace eng
