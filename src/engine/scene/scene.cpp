#pragma once

#include <vector>

#include "../engine.hpp"

class Scene {

    std::vector<eng::EntityID> entities;

    void add_entity(eng::EntityID e) { entities.push_back(e); }
    eng::EntityID emplace_entity() { return eng::Engine::instance().ecs()->create_entity(); }
};