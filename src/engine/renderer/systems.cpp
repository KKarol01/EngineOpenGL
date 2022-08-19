#include "systems.hpp"
#include "components.hpp"

#include "../engine.hpp"

RenderSystem::RenderSystem() : SystemBase() { set_component_family<RenderData>(Engine::ecs()); }

void RenderSystem::update_entity(EntityID id) {
    const auto &e       = Engine::ecs()->get_entity(id);
    const auto accepted = entity_compatible(e);

    if (!accepted && !ent_vec.contains(id)) {
        return;
    } else if (!accepted) { // if not accepted now, but earlier - cleanup
        ent_vec.at(id)->remove(id);
        ent_vec.erase(id);
        return;
    }

    const auto &rd = Engine::ecs()->get_component<RenderData>(id);

    // if registered earlier, but changed shader program - erase association
    if (ent_vec.contains(id)) { ent_vec.at(id)->remove(id); }
    // association has been removed, if new program is null - don't bother updating
    if (rd.sh == nullptr) return;

    // insert anew (if contained earlier), and udpate association
    entities[rd.sh].insert(id);
    ent_vec[id] = &entities.at(rd.sh);
}

void RenderSystem::update() {
    for (const auto &[sh, ids] : entities) {
        sh->use();
        for (const auto id : ids.data()) {
            const auto &comp = Engine::ecs()->get_component<RenderData>(id);
            comp.vao.bind();
            sh->feed_uniforms(comp.sh_datas);
            comp.vao.draw();
        }
    }
}
