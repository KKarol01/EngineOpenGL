#include "renderer.hpp"

#include "../engine.hpp"

void eng::Renderer::render() {
    pipelines.for_each([](Pipeline &p) { p.render(); });
}

void eng::Pipeline::render() {
    auto &r = *eng::Engine::instance().renderer_.get();
    for (auto &s : stages) {
        r.programs[s.program].use();
        r.vaos[s.vao].bind();
        for (auto &b : s.bufferbinders) b->bind();
        s.draw_cmd->draw();
    }
}

eng::DrawElementsCMD::DrawElementsCMD(BufferID buffer) {
    auto &r = *Engine::instance().renderer_.get();
    count   = r.buffers[buffer].descriptor.size / sizeof(uint32_t);
}

void eng::DrawElementsCMD::draw() { glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, 0); }

void eng::BufferBasedBinder::bind() {
    glBindBufferBase(gl_target, base, Engine::instance().renderer_->buffers[buffer].descriptor.handle);
}
