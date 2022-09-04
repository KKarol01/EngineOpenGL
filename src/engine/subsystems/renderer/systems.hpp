#pragma once

#include "../ecs/ecs.hpp"
#include "../../engine.hpp"

class Shader;

class RenderSystem : public eng::SystemBase {
    std::map<Shader *, eng::SortedVectorUnique<eng::EntityID>> entities;
    std::map<eng::EntityID, eng::SortedVectorUnique<eng::EntityID> *> ent_vec;

  public:
    RenderSystem();

    void update_entity(eng::EntityID) final;
    void update() final;
};