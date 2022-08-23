#include "gui.hpp"

#include "../../engine.hpp"
#include "../../wrappers/window/window.hpp"

#include <optional>
#include <variant>

#include <imgui/include_me.hpp>

GUI::GUI() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    io = &ImGui::GetIO();
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(Engine::instance().window()->glfwptr(), true);
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

void GUI::draw() const {
    ImGui_ImplGlfw_NewFrame();
    ImGui_ImplOpenGL3_NewFrame();
    ImGui::NewFrame();

    for (const auto &[_, draw] : ui_draws) { draw(); }

 //   auto s = Engine::instance().sceneinstance().();
    auto ww = Engine::instance().window()->width();
    auto wh = Engine::instance().window()->height();
    ImGui::SetNextWindowSizeConstraints({500.f, (float)wh}, {1000.f, (float)wh});
    ImGui::Begin("Scene objects", 0, ImGuiWindowFlags_NoMove);
//    for (int i = 0; i < s->object_count(); ++i) {
//        if (ImGui::CollapsingHeader(s->model_names.at(i).c_str())) {
//            ImGui::BeginChild(s->model_names.at(i).c_str(), {0, 100 + ImGui::GetFrameHeightWithSpacing()*4}, true);
//            ImGui::TextWrapped("Position");
//            ImGui::SameLine();
//            ImGui::SliderFloat3("pos", &s->model_pos_scl_rots.at(i)[0].x, -10.f, 10.f);
//            ImGui::TextWrapped("Scale");
//            ImGui::SameLine();
//            ImGui::SliderFloat3("scale", &s->model_pos_scl_rots.at(i)[1].x, -10.f, 10.f);
//            ImGui::TextWrapped("Rotation");
//            ImGui::SameLine();
//            ImGui::SliderFloat3("rot", &s->model_pos_scl_rots.at(i)[2].x, -10.f, 10.f);
//            ImGui::Separator();
//            ImGui::BeginChild("Shader Data", {0, 100}, true);
//           // for (auto &d : s->model_datas.at(i).shader_data) {
//           //     for(int j=0; j<100; ++j) {
//           //     ImGui::TextWrapped(d.name.c_str());
//           //     ImGui::SameLine();
//           //     if (auto ptr = std::get_if<int>(&d.value)) {
//           //         ImGui::SliderInt("a", ptr, -1, 1);
//           //     } else if (auto ptr = std::get_if<float>(&d.value)) {
//           //         ImGui::SliderFloat("a", ptr, -1.f, 1.f);
//           //
//           //     } else if (auto ptr = std::get_if<glm::vec2>(&d.value)) {
//           //         ImGui::SliderFloat2("a", &ptr->x, -1.f, 1.f);
//           //
//           //     } else if (auto ptr = std::get_if<glm::vec3>(&d.value)) {
//           //         ImGui::SliderFloat3("a", &ptr->x, -1.f, 1.f);
//           //     }
//           //     }
//           // }
//            ImGui::EndChild();
//            ImGui::EndChild();
//        }
//    }
    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
