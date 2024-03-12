#include "renderer.hpp"
#include <engine/engine.hpp>

eng::Renderer::Renderer() {
    auto g = Engine::instance().get_gpu_res_mgr();

    color_texture
        = g->create_resource(Texture{TextureSettings{GL_RGB16F, GL_CLAMP_TO_EDGE, GL_LINEAR, 1},
                                     TextureImageDataDescriptor{"", 1920, 1080}});
    depth_stencil_texture = g->create_resource(
        Texture{TextureSettings{GL_DEPTH24_STENCIL8, GL_CLAMP_TO_EDGE, GL_LINEAR, 1},
                TextureImageDataDescriptor{"", 1920, 1080}});
    render_fbo = Framebuffer{
        {FramebufferAttachment{GL_COLOR_ATTACHMENT0, color_texture->res_handle()},
         FramebufferAttachment{GL_DEPTH_STENCIL_ATTACHMENT, depth_stencil_texture->res_handle()}}};

    commands_buffer  = g->create_resource(GLBuffer{GL_DYNAMIC_STORAGE_BIT});
    geometry_buffer  = g->create_resource(GLBuffer{GL_DYNAMIC_STORAGE_BIT});
    index_buffer     = g->create_resource(GLBuffer{GL_DYNAMIC_STORAGE_BIT});
    mesh_data_buffer = g->create_resource(GLBuffer{GL_DYNAMIC_STORAGE_BIT});
    mesh_vao = g->create_resource(GLVao{{GLVaoBinding{0, geometry_buffer->res_handle(), 48, 0}},
                                        {GLVaoAttribute{0, 0, 3, 0},
                                         GLVaoAttribute{1, 0, 3, 0},
                                         GLVaoAttribute{2, 0, 3, 0},
                                         GLVaoAttribute{3, 0, 3, 0}},
                                        index_buffer->res_handle()});
    geometry_buffer->on_handle_change.connect([=](auto nh) { mesh_vao->update_binding(0, nh); });
    index_buffer->on_handle_change.connect([=](auto nh) { mesh_vao->update_ebo(nh); });

    quad_buffer = g->create_resource(GLBuffer{GL_DYNAMIC_STORAGE_BIT});
    {
        float quad_vertices[]{
            -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f};
        quad_buffer->push_data(quad_vertices, sizeof(quad_vertices));
    }
    quad_vao = g->create_resource(
        GLVao{{GLVaoBinding{0, quad_buffer->res_handle(), 8, 0}}, {GLVaoAttribute{0, 0, 2, 0}}});
    quad_shader = ShaderProgram{"quad"};

    bloom = new PostprocessBloom{4};
}

namespace eng {
    void MeshPass::refresh(Renderer *r) {
        auto gpu = Engine::instance().get_gpu_res_mgr();

        while (unbatched.empty() == false) {
            auto h_ro = unbatched.front();
            unbatched.erase(unbatched.begin());
            auto &ro = *gpu->get_resource(h_ro);

            PassObject po{
                PassMaterial{
                    .prog
                    = gpu->get_resource(ro.material)->passes.at(RenderPass::Forward)->res_handle()},
                ro.mesh,
                h_ro};

            pass_objects.push_back(po);
        }

        flat_batches.clear();
        indirect_batches.clear();
        multi_batches.clear();

        for (const auto &po : pass_objects) {
            auto &ro = *gpu->get_resource(po.render_object);
            flat_batches.emplace_back(get_batch_id(po.mesh, ro.material),
                                      Handle<PassObject>(po.id));
        }

        std::sort(flat_batches.begin(), flat_batches.end(), [](auto &&a, auto &&b) {
            return a.batch_id < b.batch_id;
        });

        FlatBatch *prev_fb = &flat_batches[0];
        {
            IndirectBatch ib{.mesh     = get_pass_object(prev_fb->object).mesh,
                             .material = get_pass_object(prev_fb->object).mat,
                             .first    = 0,
                             .count    = 1};
            indirect_batches.push_back(ib);
        }
        IndirectBatch *curr_ib = &indirect_batches[0];
        for (auto i = 1u; i < flat_batches.size(); ++i) {
            auto &fb = flat_batches[i];
            if (fb.batch_id != prev_fb->batch_id) {
                IndirectBatch ib{.mesh     = get_pass_object(fb.object).mesh,
                                 .material = get_pass_object(fb.object).mat,
                                 .first    = i,
                                 .count    = 1};
                indirect_batches.push_back(ib);
                curr_ib = &indirect_batches.back();
                prev_fb = &fb;
                continue;
            }
            prev_fb = &fb;
            curr_ib->count++;
        }

        multi_batches.push_back(MultiBatch{.first = 0, .count = (uint32_t)indirect_batches.size()});

        return;
    }

    uint32_t MeshPass::get_batch_id(Handle<Mesh> mesh, Handle<Material> mat) {
        auto it = std::find(_batch_ids.begin(), _batch_ids.end(), std::make_pair(mesh, mat));

        if (it == _batch_ids.end()) {
            _batch_ids.push_back(std::make_pair(mesh, mat));
            it = _batch_ids.end() - 1;
        }

        return std::distance(_batch_ids.begin(), it);
    }

    void Renderer::register_object(const Object *o) {
        auto gpu = Engine::instance().get_gpu_res_mgr();
        for (auto &m : o->meshes) {
            auto ro = gpu->create_resource(
                RenderObject{o->id, m.res_handle(), m.material, m.transform});

            _dirty_objects.emplace_back(ro->res_handle());
            _mesh_instance_count[m.id]++;

            if (gpu->get_resource(m.material)->passes.contains(RenderPass::Forward)) {
                _forward_pass.unbatched.push_back(Handle<RenderObject>{ro->res_handle()});
            }
        }
    }

    void Renderer::render() {
        auto gpu = Engine::instance().get_gpu_res_mgr();

        if (_dirty_objects.empty() == false) {
            _dirty_objects.clear();

            std::unordered_map<uint32_t, uint32_t> offsets;

            struct alignas(16) Payload {
                uint64_t diffuse;
                uint64_t normal;
                uint64_t metallic;
                uint64_t roughness;
                uint64_t emissive;
                uint64_t _1;
                glm::vec4 texture_channels;
                glm::mat4 transform;
            };

            std::vector<Payload> mesh_data;
            std::vector<uint64_t> mesh_bindless_handle;

            auto &meshes      = gpu->get_storage<Mesh>();
            auto &renderables = gpu->get_storage<RenderObject>();

            mesh_data.resize(gpu->count<RenderObject>());
            mesh_bindless_handle.resize(gpu->count<RenderObject>());

            offsets[meshes[0]->id] = 0;
            for (auto i = 1u, prev_offset = 0u; i < meshes.size(); ++i) {
                const auto &m0    = *meshes[i - 1];
                const auto &m1    = *meshes[i];
                const auto offset = _mesh_instance_count.at(m0.id) + prev_offset;
                offsets[m1.id]    = offset;
                prev_offset       = offset;
            }

            for (const auto r : gpu->get_storage<RenderObject>()) {
                const auto offset = offsets.at(r->mesh.id)++;
                const auto &mesh  = *gpu->get_resource(r->mesh);
                auto material     = gpu->get_resource(mesh.material);
                material->textures.at(TextureType::Diffuse)->make_resident();
                material->textures.at(TextureType::Normal)->make_resident();
                material->textures.at(TextureType::Metallic)->make_resident();
                material->textures.at(TextureType::Roughness)->make_resident();
                material->textures.at(TextureType::Emissive)->make_resident();

                mesh_data[offset] = {
                    material->textures.at(TextureType::Diffuse)->bindless_handle(),
                    material->textures.at(TextureType::Normal)->bindless_handle(),
                    material->textures.at(TextureType::Metallic)->bindless_handle(),
                    material->textures.at(TextureType::Roughness)->bindless_handle(),
                    material->textures.at(TextureType::Emissive)->bindless_handle(),
                    0,
                    glm::vec4{0.f, 0.f, 1.f, 0.f},
                    r->transform,
                };

                ENG_DEBUG("Inserting %i at %i\n", mesh.id, offset);
            }

            mesh_data_buffer->clear_invalidate();
            mesh_data_buffer->push_data(mesh_data.data(), mesh_data.size() * sizeof(Payload));

            std::vector<float> mesh_vertices;
            std::vector<unsigned> mesh_indices;
            for (const auto m : meshes) {
                mesh_vertices.insert(mesh_vertices.end(), m->vertices.begin(), m->vertices.end());
                mesh_indices.insert(mesh_indices.end(), m->indices.begin(), m->indices.end());
            }

            geometry_buffer->clear_invalidate();
            geometry_buffer->push_data(mesh_vertices.data(), mesh_vertices.size() * sizeof(float));

            index_buffer->clear_invalidate();
            index_buffer->push_data(mesh_indices.data(), mesh_indices.size() * sizeof(unsigned));
        }

        if (_forward_pass.unbatched.empty() == false) { _forward_pass.refresh(this); }

        struct DrawElementsIndirectCommandExtended : public DrawElementsIndirectCommand {
            DrawElementsIndirectCommandExtended() = default;
            DrawElementsIndirectCommandExtended(const DrawElementsIndirectCommand *cmd,
                                                uint32_t vertex_count) {
                *static_cast<DrawElementsIndirectCommand *>(this) = *cmd;
                this->vertex_count                                = vertex_count;
            }

            uint32_t vertex_count{0u};
        } prev_cmd;

        std::vector<DrawElementsIndirectCommand> draw_commands;

        for (auto i = 0u; i < _forward_pass.indirect_batches.size(); ++i) {
            const auto &ib = _forward_pass.indirect_batches[i];
            const auto &m  = *gpu->get_resource(ib.mesh);
            DrawElementsIndirectCommand cmd{
                .count          = (uint32_t)m.indices.size(),
                .instance_count = ib.count,
                .first_index    = prev_cmd.first_index + prev_cmd.count,
                .base_vertex    = prev_cmd.base_vertex + prev_cmd.vertex_count,
                .base_instance  = prev_cmd.base_instance + prev_cmd.instance_count};
            prev_cmd = DrawElementsIndirectCommandExtended{&cmd, (uint32_t)m.vertices.size() / 12u};
            draw_commands.push_back(cmd);
        }

        commands_buffer->clear_invalidate();
        commands_buffer->push_data(draw_commands.data(),
                                   draw_commands.size() * sizeof(DrawElementsIndirectCommand));

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);

        render_fbo.bind();
        mesh_vao->bind();
        mesh_data_buffer->bind_base(GL_SHADER_STORAGE_BUFFER, 0);
        commands_buffer->bind(GL_DRAW_INDIRECT_BUFFER);
        glViewport(0, 0, 1920, 1080);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        for (const auto &mb : _forward_pass.multi_batches) {
            auto prog = gpu->get_resource(_forward_pass.indirect_batches[mb.first].material.prog);
            prog->use();
            prog->set("v", Engine::instance().get_camera()->view_matrix());
            prog->set("p", Engine::instance().get_camera()->perspective_matrix());
            prog->set("view_vec", Engine::instance().get_camera()->forward_vec());
            prog->set("view_pos", Engine::instance().get_camera()->position());
            prog->use();
            int draw_count = mb.first * sizeof(DrawElementsIndirectCommand);
            glMultiDrawElementsIndirect(
                GL_TRIANGLES, GL_UNSIGNED_INT, (void *)draw_count, mb.count, 0);
        }

        bloom->render(color_texture, quad_vao);

        glViewport(0, 0, 1920, 1080);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        quad_shader.use();
        quad_vao->bind();
        color_texture->bind(0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }

} // namespace eng
