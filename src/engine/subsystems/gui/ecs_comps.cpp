#include "ecs_comps.hpp"

#include "../../engine.hpp"
#include "../ecs/ecs.hpp"
#include "../ecs/components.hpp"
#include "../../wrappers/shader/shader.hpp"

#include <imgui/imgui.h>
#include <glm/glm.hpp>

void ComponentGUI::draw_entity(eng::EntityID eid) {
    auto ecs    = eng::Engine::instance().ecs();
    auto entity = ecs->get_entity(eid);

    

    if (entity && entity.value().get().contains(ecs->get_comp_id<RenderData>())) {

        ecs->modify_component<RenderData>(eid, [&](RenderData &data) -> void {
            for (auto &sh_data : data.sh_datas.data) {

            }
        });
    }
}
