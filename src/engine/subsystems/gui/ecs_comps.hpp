#pragma once

#include <functional>

#include "../ecs/ecs.d.hpp"

class ComponentGUI {
  public:
	void draw_entity(eng::EntityID eid);
};