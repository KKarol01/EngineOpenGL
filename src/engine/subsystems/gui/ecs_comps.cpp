#include "ecs_comps.hpp"

#include "../../engine.hpp"
#include "../ecs/ecs.hpp"
#include "../ecs/components.hpp"

#include <imgui/imgui.h>

void ComponentGUI::draw_entity(EntityID eid) {
    auto ecs    = Engine::instance().ecs();
    auto entity = ecs->get_entity(eid);

    if (entity.contains(ecs->get_comp_id<RenderData>())) {
        auto &rd = ecs->get_component<RenderData>(eid);
        ImGui::TextWrapped("name: %s", rd.sh_datas[0].name.c_str());
    }
}
