#include "ecs.d.hpp"


eng::EntityID eng::ECS::create_entity() {
    static EntityID gid{0u};
    entities[gid];
    return gid++;
}

eng::OptionalReference<eng::Entity> eng::ECS::get_entity(eng::EntityID eid) { return entities.at(eid); }

void eng::ECS::update_entity(eng::EntityID id) {
    for (auto &s : systems) s.second->update_entity(id);
}

void eng::ECS::_update() {
    for (auto &s : systems) s.second->_update();
}

eng::ECS::~ECS() {
    for (auto &s : systems) delete s.second;
}