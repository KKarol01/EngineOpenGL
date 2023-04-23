#include "renderer.hpp"

#include "../engine.hpp"

eng::Renderer::Renderer() {
    glCreateVertexArrays(1, &vao);

    glVertexArrayAttribBinding(vao, 0, 0);
    glVertexArrayAttribFormat(vao, 0, 3, GL_FLOAT, GL_FALSE, 0);
    glEnableVertexArrayAttrib(vao, 0);
}

namespace eng {
    Handle<RenderObject> Renderer::register_object(const Object *object) {
        for (auto &m : object->meshes) {
            RenderObject ro{
                .material = get_material_handle(m.material), .mesh = get_mesh_handle(&m), .transform = m.transform};

            auto &r = renderables.emplace_back(Handle<RenderObject>::get(), ro);
            newly_added_objects.push_back(r.first);

            if (m.use_forward_pass && m.material->pass->pipelines.contains(PipelinePass::Forward)) {
                forward_pass.unbatched.push_back(r.first);
                forward_pass.needs_refresh = true;
            }
        }

        return Handle<RenderObject>{0};
    }

    Handle<Material> Renderer::get_material_handle(const Material *mat) {
        for (auto &[h, material] : materials) {
            if (material.pass == mat->pass) { return h; }
        }

        return materials.emplace_back(Handle<Material>::get(), Material{*mat}).first;
    }

    Handle<Mesh> Renderer::get_mesh_handle(const Mesh *m) {
        for (auto &[h, mesh] : meshes) {
            if (mesh.id == m->id) { return h; }
        }

        auto handle = meshes.push_back(Mesh{*m});
        nongpu_resident_meshes.push_back(handle);
        return handle;
    }

    void Renderer::render() {

        if (newly_added_objects.empty() == false) {
            std::vector<glm::mat4> transforms;

            while (newly_added_objects.empty() == false) {
                auto obj_handle = newly_added_objects.front();
                newly_added_objects.erase(newly_added_objects.begin());
                auto obj = get_render_object(obj_handle);
                transforms.push_back(obj->transform);
            }

            ssbo.push_data(transforms.data(), transforms.size() * sizeof(glm::mat4));
        }

        if (nongpu_resident_meshes.empty() == false) {
            std::vector<float> floats;
            std::vector<unsigned> indices;

            while (nongpu_resident_meshes.empty() == false) {
                auto mesh_handle = nongpu_resident_meshes.front();
                nongpu_resident_meshes.erase(nongpu_resident_meshes.begin());
                auto &mesh = meshes[mesh_handle];

                floats.insert(floats.end(), mesh.vertices.begin(), mesh.vertices.end());
                indices.insert(indices.end(), mesh.indices.begin(), mesh.indices.end());

                auto &mbls = mesh_buffer_locations;

                MeshBufferLocation mbl{
                    .mesh        = mesh_handle,
                    .first_index = (mbls.empty() == false) ? mbls.back().first_index + mbls.back().index_count : 0,
                    .index_count = (uint32_t)mesh.indices.size(),
                    .first_vertex
                    = (mbls.empty() == false) ? (mbls.back().first_vertex + (int)mbls.back().vertex_count) : 0,
                    .vertex_count = (uint32_t)mesh.vertices.size() / 3};

                mesh_buffer_locations.push_back(mbl);
            }

            geometry_buffer.push_data(floats.data(), floats.size() * sizeof(float));
            index_buffer.push_data(indices.data(), indices.size() * sizeof(unsigned));

            glVertexArrayVertexBuffer(vao, 0, geometry_buffer.descriptor.handle, 0, 12);
            glVertexArrayElementBuffer(vao, index_buffer.descriptor.handle);
        }

        if (forward_pass.needs_refresh) { forward_pass.refresh(this); }

        draw_commands.clear();
        for (const auto &mb : forward_pass.multibatches) {
            for (uint32_t i = mb.first; i < mb.first + mb.count; ++i) {
                const auto &mi   = forward_pass.indirectbatches[i];
                const auto &mesh = meshes[mi.mesh];
                const auto &mbl  = *std::find(mesh_buffer_locations.begin(), mesh_buffer_locations.end(), mi.mesh);

                DrawElementsIndirectCommand cmd{.count         = mbl.index_count,
                                                .instanceCount = mi.count,
                                                .firstIndex    = mbl.first_index,
                                                .baseVertex    = mbl.first_vertex,
                                                .baseInstance  = (uint32_t)draw_commands.size()};

                draw_commands.push_back(cmd);
            }
        }

        draw_buffer.clear_invalidate();
        draw_buffer.push_data(draw_commands.data(), draw_commands.size() * sizeof(DrawElementsIndirectCommand));

        glBindVertexArray(vao);
        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, draw_buffer.descriptor.handle);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo.descriptor.handle);
        for (const auto &mb : forward_pass.multibatches) {
            auto program = forward_pass.indirectbatches[mb.first].material.program;
            program->use();
            program->set("v", Engine::instance().cam->view_matrix());
            program->set("p", Engine::instance().cam->perspective_matrix());

            glMultiDrawElementsIndirect(
                GL_TRIANGLES, GL_UNSIGNED_INT, (void *)(mb.first * sizeof(DrawElementsIndirectCommand)), mb.count, 0);
        }
    }

} // namespace eng

void eng::MeshPass::refresh(Renderer *r) {
    needs_refresh = false;

    while (unbatched.empty() == false) {
        const auto rh = *unbatched.begin();
        const auto ro = r->get_render_object(rh);
        unbatched.erase(unbatched.begin());

        PassObject po{.material
                      = PassMaterial{r->get_material(ro->material)->pass->pipelines.at(PipelinePass::Forward)},
                      .mesh     = ro->mesh,
                      .original = rh};

        auto po_handle = objects.push_back(po);
    }

    flatbatches.clear();
    indirectbatches.clear();
    multibatches.clear();

    for (const auto &po : objects) {
        RenderBatch rb{.object = po.first, .id = assign_batch_id(*r->get_render_object(po.second.original))};
        flatbatches.push_back(rb);
    }

    std::sort(flatbatches.begin(), flatbatches.end(), [](auto &&a, auto &&b) { return a.id < b.id; });

    PassMaterial prev_mat;
    uint32_t prev_handle;
    for (auto i = 0ull; i < flatbatches.size(); ++i) {
        auto &obj     = objects[flatbatches[i].object];
        auto material = obj.material;
        auto mesh     = obj.mesh;

        if (i == 0ull || (material.program != prev_mat.program || prev_handle != mesh.handle)) {
            indirectbatches.emplace_back();
            indirectbatches.back().first    = i;
            indirectbatches.back().count    = 1;
            indirectbatches.back().material = material;
            indirectbatches.back().mesh     = mesh;
            prev_mat.program                = material.program;
            prev_handle                     = mesh.handle;
            continue;
        }

        indirectbatches.back().count++;
    }

    multibatches.emplace_back(0, indirectbatches.size());
}

uint32_t eng::MeshPass::assign_batch_id(const RenderObject &ro) {
    const auto p = std::make_pair(ro.mesh, ro.material);

    if (batch_ids.contains(p) == false) {
        batch_ids[p] = (uint32_t)(Handle<std::pair<Handle<Mesh>, Handle<Material>>>::get());
    }

    return batch_ids.at(p);
}
