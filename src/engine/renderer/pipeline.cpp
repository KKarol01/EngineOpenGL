#include "pipeline.hpp"

#include <cassert>

#include "../engine.hpp"
#include "./renderer.hpp"

#include <glad/glad.h>

RenderingPipeline::RenderingPipeline() {
    auto &r = *eng::Engine::instance().renderer_.get();

    vao               = r.create_vao();
    geometry_vertices = r.create_buffer(0u);
    geometry_indices  = r.create_buffer(0u);
    textures_ssbo     = r.create_buffer(0u);
}

void RenderingPipeline::allocate_model(const Model &m) {
    auto it    = get_record(m.id);
    auto found = it != records.end();

    if (found) return;

    auto offset = records.size() == 0 ? 0 : records.back().offset + records.back().vertices_size_bytes;
    it          = records.emplace(it, m, offset);

    std::vector<float> vertices;
    std::vector<unsigned> indices;

    for (auto &mesh : m.meshes) {
        vertices.insert(vertices.end(), mesh.vertices.begin(), mesh.vertices.end());
        indices.insert(indices.end(), mesh.indices.begin(), mesh.indices.end());
    }

    auto &gv = eng::Engine::instance().renderer_->get_buffer(geometry_vertices);
    auto &gi = eng::Engine::instance().renderer_->get_buffer(geometry_indices);
    gv.push_data(vertices.data(), vertices.size() * 4);
    gi.push_data(indices.data(), indices.size() * 4);

    auto &buff = eng::Engine::instance().renderer_->get_buffer(textures_ssbo);

    std::vector<uint64_t> handles;
    for (auto &mesh : m.meshes) {
        handles.push_back(mesh.material->arr[0].bindless_handle);
        handles.push_back(mesh.material->arr[1].bindless_handle);
        handles.push_back(mesh.material->arr[2].bindless_handle);
        handles.push_back(mesh.material->arr[3].bindless_handle);
    }

    buff.push_data(handles.data(), handles.size() * 8);
    vvv = buff.descriptor.handle;
}

void RenderingPipeline::create_instance(uint32_t mid) {
    auto record = get_record(mid);
    if (record == records.end()) return;

    on_geometry_add.emit(mid, *record);
}

void RenderingPipeline::render() {
    auto &re = *eng::Engine::instance().renderer_.get();

    for (auto &phase : schedule.phases) {
        for (auto &stage : phase) {
          //  glUseProgram(stage.pid);
            re.get_vao(stage.vid).bind();
            stage.cmd->draw();
        }
    }
}
