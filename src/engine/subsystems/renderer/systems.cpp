#include "systems.hpp"
#include "../ecs/components.hpp"

#include "../../engine.hpp"

RenderSystem::RenderSystem() : eng::SystemBase() { set_component_family<RenderData>(eng::Engine::instance().ecs()); }

void RenderSystem::update_entity(eng::EntityID id) {
    const auto &e       = eng::Engine::instance().ecs()->get_entity(id);

    if(!e) return;

    const auto accepted = entity_compatible(e.value().get());

    if (!accepted && !ent_vec.contains(id)) {
        return;
    } else if (!accepted) {
        ent_vec.at(id)->remove(id);
        ent_vec.erase(id);
        return;
    }

    const auto &rd = eng::Engine::instance().ecs()->get_component<RenderData>(id);

    if (ent_vec.contains(id)) { ent_vec.at(id)->remove(id); }
    if (rd.sh == nullptr) return;

    entities[rd.sh].insert(id);
    ent_vec[id] = &entities.at(rd.sh);
}

void RenderSystem::update() {
    for (const auto &[sh, ids] : entities) {
        sh->use();
        for (const auto id : ids.data()) {
            const auto &comp = eng::Engine::instance().ecs()->get_component<RenderData>(id);
            sh->feed_uniforms(comp.sh_datas);
            comp.vao.bind();
            comp.vao.draw();
        }
    }
}
