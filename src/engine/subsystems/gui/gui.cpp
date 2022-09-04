#include "../../engine.hpp"
#include "../ecs/ecs.hpp"
#include "../../wrappers/window/window.hpp"

#include "gui.hpp"

#include <optional>
#include <variant>

#include <imgui/include_me.hpp>

GUI::GUI() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    io = &ImGui::GetIO();
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(eng::Engine::instance().window()->glfwptr(), true);
    ImGui_ImplOpenGL3_Init("#version 460 core");

    io->IniFilename         = "imgui.ini";
    io->WantSaveIniSettings = true;
    io->IniSavingRate       = 2.f;
}

GUI::~GUI() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();
}

void GUI::draw() {
    ImGui_ImplGlfw_NewFrame();
    ImGui_ImplOpenGL3_NewFrame();
    ImGui::NewFrame();

    for (const auto &[_, draw] : ui_draws) { draw(); }

    //   auto s = Engine::instance().sceneinstance().();
    auto ww = eng::Engine::instance().window()->width();
    auto wh = eng::Engine::instance().window()->height();
    ImGui::SetNextWindowSizeConstraints({500.f, (float)wh}, {1000.f, (float)wh});
    ImGui::Begin("Scene objects", 0, ImGuiWindowFlags_NoMove);

    const auto &objs = eng::Engine::instance().ecs()->get_entities();

    for (std::uint32_t i = 0u; i < objs.size(); ++i) {
        const auto &entity = objs.at(i);
        if (ImGui::CollapsingHeader(std::to_string(i).c_str())) {
            ImGui::BeginChild(std::to_string(i).c_str(), {0, 100 + ImGui::GetFrameHeightWithSpacing() * 4}, true);
            comp_gui.draw_entity(entity.first);
            ImGui::EndChild();
        }
    }
    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
