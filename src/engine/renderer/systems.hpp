#pragma once

#include "../ecs/ecs.hpp"
#include "../engine.hpp"

class RenderSystem : public SystemBase {
    std::map<Shader *, SortedVectorUnique<EntityID>> entities;
    std::map<EntityID, SortedVectorUnique<EntityID> *> ent_vec;

  public:
    RenderSystem();

    void update_entity(EntityID) final;
    void update() final;
};