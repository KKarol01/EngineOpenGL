#include "ecs.hpp"

void ECS::update_entity(EntityID id) {
    for (auto &s : systems) s.second->update_entity(id);
}

void ECS::update() {
    for (auto &s : systems) s.second->update();
}

ECS::~ECS() {
    for (auto &s : systems) delete s.second;
}