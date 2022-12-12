#include "renderer.hpp"

#include <ranges>

#include "../engine.hpp"

void eng::Renderer::render() {
    pipelines.for_each([](Pipeline &p) { p.render(); });
}

eng::Pipeline::Pipeline() {
    auto &r = *Engine::instance().renderer_.get();
    vbo     = r.buffers.emplace();
    ebo     = r.buffers.emplace();
}

eng::Pipeline::Pipeline(ModelPipelineAdapter adapter) : Pipeline() { this->adapter = adapter; }

void eng::Pipeline::render() {
    auto &r = *eng::Engine::instance().renderer_.get();
    for (auto &s : stages) {
        if (s.program != 0) r.programs[s.program].use();
        if (s.vao != 0) r.vaos[s.vao].bind();
        for (auto &b : s.bufferbinders) b->bind();
        if (s.on_stage_start) s.on_stage_start();
        s.draw_cmd->draw();
        if (s.on_stage_end) s.on_stage_end();
    }
}

void eng::Pipeline::add_model(const Model &m) {
    auto &r = *Engine::instance().renderer_.get();

    auto vbo_data = adapter.convert(m);

    on_model_add.emit(m);
    r.buffers[vbo].push_data(vbo_data.data(), vbo_data.size() * sizeof(float));
    r.buffers[ebo].push_data(m.indices.data(), m.indices.size() * sizeof(unsigned));
}

eng::DrawElementsCMD::DrawElementsCMD(BufferID buffer) {
    auto &r = *Engine::instance().renderer_.get();
    count   = r.buffers[buffer].descriptor.size / sizeof(uint32_t);
}

void eng::DrawElementsCMD::draw() { glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, 0); }

void eng::BufferBasedBinder::bind() {
    glBindBufferBase(gl_target, base, Engine::instance().renderer_->buffers[buffer].descriptor.handle);
}

eng::ModelPipelineAdapter::ModelPipelineAdapter(std::initializer_list<ATTRIBUTES> vbo_layout) : layout{vbo_layout} {
    assert(vbo_layout.size() > 0);

    for (auto l : vbo_layout) {
        switch (l) {
        case eng::ModelPipelineAdapter::ATTR_POSITION:
        case eng::ModelPipelineAdapter::ATTR_NORMAL: stride += 3; break;
        case eng::ModelPipelineAdapter::ATTR_TEXCOORDS: stride += 2;
        default: throw std::runtime_error{"Wrong attribute"};
        }
    }
}

std::vector<float> eng::ModelPipelineAdapter::convert(const Model &model) {

    auto add_position = std::find(layout.begin(), layout.end(), ATTR_POSITION) != layout.end();

    std::vector<float> model_data;

    for (const auto &m : model.meshes) {

        if (add_position) {
            for (auto v : m.vertices) {
                model_data.push_back(model.vertices[v].x);
                model_data.push_back(model.vertices[v].y);
                model_data.push_back(model.vertices[v].z);
            }
        }
    }

    assert(model_data.size() > 0);

    return model_data;
}

eng::DrawElementsInstancedCMD::DrawElementsInstancedCMD(BufferID buffer, uint32_t instances) : instances{instances} {}

void eng::DrawElementsInstancedCMD::draw() {}

eng::DrawCMD::DrawCMD() { gl_mode = GL_TRIANGLES; }

eng::DrawCMD::DrawCMD(uint32_t gl_mode) { this->gl_mode = gl_mode; }

void eng::DrawArraysInstancedCMD::draw() { glDrawArraysInstanced(gl_mode, first, vertex_count, instance_count); }

void eng::DrawArraysInstancedBaseInstanceCMD::draw() {
    glDrawArraysInstancedBaseInstance(gl_mode, first, vertex_count, instance_count, base_instance);
}
