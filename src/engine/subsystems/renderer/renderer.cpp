#include "renderer.hpp"

#include "systems.hpp"
#include "../ecs/components.hpp"

#include "../../engine.hpp"
#include "renderer.hpp"

static eng::SystemID render_system_id{0u};
static RenderSystem *rs{nullptr};


Renderer::Renderer() {
    eng::Engine::instance().ecs()->register_component<RenderData>();
    render_system_id = eng::Engine::instance().ecs()->add_system<RenderSystem>();
    rs = eng::Engine::instance().ecs()->get_system<RenderSystem>(render_system_id);
}

void Renderer::render_frame() {
    rs->update(); 
}