#include "renderer.hpp"

#include <ranges>

#include "../engine.hpp"

eng::Renderer::Renderer() {
    glCreateVertexArrays(1, &vao);

    glVertexArrayAttribBinding(vao, 0, 0);
    glVertexArrayAttribFormat(vao,0,3,GL_FLOAT, GL_FALSE, 0);
    glEnableVertexArrayAttrib(vao, 0);
}

void eng::MeshPass::refresh(Renderer *r) {
    needs_refresh = false;

    std::unordered_map<uint32_t, uint32_t> po_bid;

    while (unbatched.empty() == false) {
        const auto rh  = *unbatched.begin();
        const auto &ro = r->get_render_object(rh);
        unbatched.erase(unbatched.begin());

        PassObject po;
        po.material   = PassMaterial{r->get_material(ro.material)->original->passes.at(PipelinePass::Forward)};
        po.mesh       = ro.mesh;
        po.original   = rh;
        po.custom_key = objects.size() + 1;
        objects.push_back(po);
        po_bid[po.custom_key] = assign_batch_id(ro);
    }

    flatbatches.clear();
    indirectbatches.clear();
    multibatches.clear();

    for (const auto &po : objects) {
        RenderBatch rb;
        rb.id     = po_bid.at(po.custom_key);
        rb.object = Handle<PassObject>{po.custom_key};
        flatbatches.push_back(rb);
    }

    std::sort(flatbatches.begin(), flatbatches.end(), [](auto &&a, auto &&b) { return a.id < b.id; });

    PassMaterial prev_mat;
    uint32_t prev_handle;
    for (auto i = 0ull; i < flatbatches.size(); ++i) {
        auto material = objects[flatbatches[i].object.handle - 1].material;
        auto mesh     = objects[flatbatches[i].object.handle - 1].mesh;

        if (i == 0ull || (material.prog != prev_mat.prog || prev_handle != mesh.handle)) {
            indirectbatches.emplace_back();
            indirectbatches.back().first    = i;
            indirectbatches.back().count    = 1;
            indirectbatches.back().material = material;
            indirectbatches.back().mesh     = mesh;
            prev_mat.prog                   = material.prog;
            prev_handle                     = mesh.handle;
            continue;
        }

        indirectbatches.back().count++;
    }

    multibatches.emplace_back(0, indirectbatches.size());
}

uint32_t eng::MeshPass::assign_batch_id(const RenderObject &ro) {
    const auto p  = std::make_pair(ro.mesh.handle, ro.material.handle);
    const auto it = std::find(batch_ids.cbegin(), batch_ids.cend(), p);

    if (it == batch_ids.cend()) {
        batch_ids.push_back(p);
        return batch_ids.size();
    }

    return std::distance(batch_ids.cbegin(), it) + 1;
}
