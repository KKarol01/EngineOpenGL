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
                std::visit(ExhaustiveVisitor{[&](auto &&val) {
                               if constexpr (std::is_convertible_v<decltype(val), glm::vec3>) {
                                   ImGui::SliderFloat3(sh_data.first.c_str(), &val.get().x, 0.f, 1.f);
                               }
                           }},
                           sh_data.second());
            }
        });
    }
}
