#include "renderer.hpp"

#include "systems.hpp"
#include "components.hpp"

#include "../engine.hpp"
static SystemID render_system_id{0u};
static RenderSystem *rs{nullptr};

Renderer::Renderer() {
    Engine::ecs()->register_component<RenderData>();
    render_system_id = Engine::ecs()->add_system<RenderSystem>();
    rs = Engine::ecs()->get_system<RenderSystem>(render_system_id);
}

void Renderer::render_frame() { rs->update(); }
