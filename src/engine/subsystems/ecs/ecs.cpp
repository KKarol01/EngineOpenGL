#include "ecs.d.hpp"

EntityID ECS::create_entity() {
    static EntityID gid{0u};
    entities[gid];
    return gid++;
}

Entity &ECS::get_entity(EntityID eid) { return entities.at(eid); }

void ECS::update_entity(EntityID id) {
    for (auto &s : systems) s.second->update_entity(id);
}

void ECS::update() {
    for (auto &s : systems) s.second->update();
}

ECS::~ECS() {
    for (auto &s : systems) delete s.second;
}