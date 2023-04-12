#pragma once

#include <cstdint>
#include <vector>
#include <concepts>
#include <unordered_map>

#include "../types/idallocator.hpp"

#include "typedefs.hpp"

#include "../types/shared_resource.hpp"
#include "../gpu/buffers/buffer.hpp"
#include "../gpu/shaderprogram/shader.hpp"
#include "glm/glm.hpp"

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
        static auto get() { return Handle<T>{HandleCounter<T>::request_handle()}; }
        uint32_t handle{0u};
    };
    enum class PipelinePass { Forward };
    using PerPassData = std::unordered_map<PipelinePass, ShaderProgram *>;

    struct MaterialPass {
        PerPassData passes;
    };

    struct Material {
        MaterialPass *original;
    };

    struct VertexLayout {
        uint32_t data_type_size_bytes{sizeof(1.f)};
        uint32_t stride{0u};
    };

    struct Mesh {
        uint32_t sortkey{0u};
        std::vector<float> vertices;
        std::vector<unsigned> indices;
        VertexLayout layout;
        Material *material{nullptr};
        glm::mat4 transform{1.f};
        bool use_forward_pass{true};
    };

    struct RenderObject;

    struct PassMaterial {
        ShaderProgram *prog;
    };
    struct PassObject {
        PassMaterial material;
        Handle<Mesh> mesh;
        Handle<RenderObject> original;
        uint32_t custom_key;
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

        std::vector<PassObject> objects;

        std::vector<MultiBatch> multibatches;
        std::vector<IndirectBatch> indirectbatches;
        std::vector<RenderBatch> flatbatches;

        std::vector<Handle<RenderObject>> unbatched;

        bool needs_refresh{false};

      private:
        uint32_t assign_batch_id(const RenderObject &ro);

        std::vector<std::pair<uint32_t, uint32_t>> batch_ids;
    };

    struct SceneObject {
        std::vector<Mesh> meshes;
    };

    struct RenderObject {
        uint32_t sortkey;
        Handle<Material> material;
        Handle<Mesh> mesh;
        glm::mat4 transform;
    };

    class Renderer {
      public:
        Renderer();

        Handle<RenderObject> register_object(Mesh *mesh) {
            RenderObject robj;
            robj.sortkey   = mesh->sortkey;
            robj.material  = get_material_handle(mesh->material);
            robj.mesh      = get_mesh_handle(mesh);
            robj.transform = mesh->transform;
            robj.sortkey   = mesh->sortkey;

            renderables.push_back(std::move(robj));
            auto handle = Handle<RenderObject>{static_cast<uint32_t>(renderables.size())};

            if (mesh->use_forward_pass) {
                if (mesh->material->original->passes.contains(PipelinePass::Forward)) {
                    // add to pass
                    forward_pass.unbatched.push_back(handle);
                    forward_pass.needs_refresh = true;
                }
            }

            dirty_objects.push_back(handle);

            return handle;
        }

        void render() {
            while (dirty_objects.empty() == false) {
                auto h = dirty_objects.front();
                dirty_objects.erase(dirty_objects.begin());
                auto &ro = renderables[h.handle - 1];
                auto &m  = meshes[ro.mesh.handle - 1];
                MeshDrawInfo minfo;
                minfo.first_index   = mesh_buffer_location.size() == 0 ? 0
                                                                       : mesh_buffer_location.back().first_index
                                                                           + mesh_buffer_location.back().index_count;
                minfo.index_count   = m.indices.size();
                minfo.vertex_offset = mesh_buffer_location.size() == 0 ? 0
                                                                       : mesh_buffer_location.back().vertex_offset
                                                                             + mesh_buffer_location.back().vertex_count;
                minfo.vertex_count  = m.vertices.size() / 3;

                mesh_buffer_location.push_back(minfo);
                geometry_buffer.push_data(m.vertices.data(), m.vertices.size() * sizeof(float));
                index_buffer.push_data(m.indices.data(), m.indices.size() * sizeof(unsigned));

                glVertexArrayVertexBuffer(vao, 0, geometry_buffer.descriptor.handle, 0, 12);
                glVertexArrayElementBuffer(vao, index_buffer.descriptor.handle);
            }

            if (forward_pass.needs_refresh) { forward_pass.refresh(this); }

            draw_buffer.clear_invalidate();
            commands.clear();

            for (const auto &m : forward_pass.multibatches) {
                DrawElementsIndirectCommand cmd;

                for (auto i = m.first; i < m.first + m.count; ++i) {
                    const auto &in  = forward_pass.indirectbatches[i];
                    const auto &mbl = mesh_buffer_location[in.mesh.handle - 1];

                    cmd.firstIndex    = mbl.first_index;
                    cmd.baseVertex    = mbl.vertex_offset;
                    cmd.baseInstance  = 0;
                    cmd.count         = mbl.index_count;
                    cmd.instanceCount = in.count;
                    commands.push_back(cmd);
                }
            }

            draw_buffer.push_data(commands.data(), commands.size() * sizeof(DrawElementsIndirectCommand));

            materials[0]->original->passes.at(PipelinePass::Forward)->use();
            glBindVertexArray(vao);
            glBindBuffer(GL_DRAW_INDIRECT_BUFFER, draw_buffer.descriptor.handle);
            glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, 0, commands.size(), 0);
        }

        RenderObject &get_render_object(Handle<RenderObject> h) { return renderables[h.handle - 1]; }
        Material *get_material(Handle<Material> h) { return materials[h.handle - 1]; }

      private:
        Handle<Material> get_material_handle(Material *mat) {
            auto it = std::find_if(
                materials.cbegin(), materials.cend(), [mat](auto &&val) { return mat->original == val->original; });

            if (it == materials.cend()) {
                materials.push_back(mat);
                it = materials.cend() - 1;
            }

            return Handle<Material>{static_cast<uint32_t>(std::distance(materials.cbegin(), it) + 1)};
        }

        Handle<Mesh> get_mesh_handle(Mesh *mesh) {
            auto it = std::find_if(
                meshes.cbegin(), meshes.cend(), [mesh](auto &&val) { return val.sortkey == mesh->sortkey; });

            if (it == meshes.cend()) {
                meshes.push_back(*mesh);
                it = meshes.cend() - 1;
            }

            return Handle<Mesh>{static_cast<uint32_t>(std::distance(meshes.cbegin(), it) + 1)};
        }

        MeshPass forward_pass;

        std::vector<RenderObject> renderables;
        std::vector<Mesh> meshes;
        std::vector<Material *> materials;

        struct MeshDrawInfo {
            uint32_t vertex_offset, vertex_count;
            uint32_t first_index, index_count;
        };
        typedef struct {
            uint32_t count;
            uint32_t instanceCount;
            uint32_t firstIndex;
            int baseVertex;
            uint32_t baseInstance;
        } DrawElementsIndirectCommand;

        std::vector<DrawElementsIndirectCommand> commands;

        std::vector<MeshDrawInfo> mesh_buffer_location;

        std::vector<Handle<RenderObject>> dirty_objects;
        GLBuffer geometry_buffer{GL_DYNAMIC_STORAGE_BIT}, index_buffer{GL_DYNAMIC_STORAGE_BIT};
        GLBuffer draw_buffer{GL_DYNAMIC_STORAGE_BIT};

        uint32_t vao;
    };

} // namespace eng