#include "3dcalc.hpp"

#include <vector>
#include <string>
#include <functional>
#include <filesystem>
#include <fstream>

#include <imgui/imgui.h>
#include <imgui/imgui_stdlib.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "../engine/engine.hpp"

//Graph3D::Graph3D() {
//    auto &engine = eng::Engine::instance();
//    auto window  = engine.window();
//
//    default_program = engine.renderer_->create_program();
//    make_program();
//    auto vao = engine.renderer_->create_vao();
//    auto vbo = engine.renderer_->create_buffer();
//    auto ebo = engine.renderer_->create_buffer();
//
//    {
//        vao->configure_binding(0, vbo, 8, 0);
//        vao->configure_ebo(ebo);
//        vao->configure_attribute(eng::GL_ATTR_0, 0, 2, 0);
//
//        uint32_t indices[4]{0, 2, 1, 3};
//        glm::vec2 vertices[4]{
//            {0.f, 0.f},
//            {1.f, 0.f},
//            {0.f, 1.f},
//            {1.f, 1.f},
//        };
//
//        vbo->push_data(vertices, sizeof(vertices));
//        ebo->push_data(indices, sizeof(indices));
//    }
//
//    auto &renderer      = *engine.renderer_;
//    pipeline             = renderer.create_pipeline();
//    auto &pps1          = pipeline->create_stage();
//    pps1.program        = default_program;
//    pps1.vao            = vao;
//    pps1.draw_cmd       = std::make_shared<eng::DrawElementsInstancedCMD>(vbo, 1000000);
//    pps1.on_stage_start = [&]() {
//        auto &p = eng::Engine::instance().renderer_->get_program(default_program);
//        p.set("model", glm::mat4{1.f});
//        p.set("view", engine.cam->view_matrix());
//        p.set("projection", engine.cam->perspective_matrix());
//        p.set("TIME", (float)glfwGetTime());
//        constants[0].value = std::to_string((float)glfwGetTime());
//
//        if (wireframe) {
//            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
//        } else {
//            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
//        }
//    };
//
//    glCreateFramebuffers(1, &fbo);
//    glCreateTextures(GL_TEXTURE_2D, 1, &coltex);
//    glCreateTextures(GL_TEXTURE_2D, 1, &depthtex);
//    glTextureParameteri(coltex, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//    glTextureParameteri(coltex, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//    glTextureParameteri(coltex, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//    glTextureParameteri(coltex, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//    glTextureStorage2D(coltex, 1, GL_RGBA8, window->width() * .75f, window->height() * .75f);
//    glTextureParameteri(depthtex, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//    glTextureParameteri(depthtex, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//    glTextureParameteri(depthtex, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//    glTextureParameteri(depthtex, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//    glTextureStorage2D(depthtex, 1, GL_DEPTH24_STENCIL8, window->width() * .75f, window->height() * .75f);
//
//    glNamedFramebufferTexture(fbo, GL_COLOR_ATTACHMENT0, coltex, 0);
//    glNamedFramebufferTexture(fbo, GL_DEPTH_STENCIL_ATTACHMENT, depthtex, 0);
//
//    assert(glCheckNamedFramebufferStatus(fbo, GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
//
//    engine.gui_.get()->add_draw(std::bind(&Graph3D::draw_gui, this));
//}
//
//void Graph3D::render() {}
//
//void Graph3D::draw_gui() {
//    auto window   = eng::Engine::instance().window();
//    auto window_w = static_cast<float>(window->width());
//    auto window_h = static_cast<float>(window->height());
//
//    ImGui::SetNextWindowPos(ImVec2(0, 0));
//    ImGui::SetNextWindowSize(ImVec2((float)window->width(), (float)window->height()));
//    if (ImGui::Begin("##main_window", 0, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDecoration)) {
//        if (ImGui::BeginMenuBar()) {
//            if (ImGui::Button("Open project")) {}
//            if (ImGui::Button("Save project")) {}
//            if (ImGui::Button("Save project as")) {}
//            ImGui::Checkbox("Wireframe mode", &wireframe);
//            ImGui::EndMenuBar();
//        }
//
//        if (ImGui::BeginChild("##render", ImVec2(window_w * .75f, window_h * .75f), true)) {
//            ImGui::Image((void *)coltex, ImGui::GetContentRegionAvail(), ImVec2(0, 1), ImVec2(1, 0));
//        }
//        ImGui::EndChild();
//        auto cpos = ImGui::GetCursorPos();
//
//        ImGui::SameLine();
//
//        if (ImGui::BeginChild("##equation", ImVec2(0, 0), true, ImGuiWindowFlags_NoScrollbar)) {
//            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
//            if (ImGui::BeginCombo("##Add...", "Add...", ImGuiComboFlags_NoArrowButton | ImGuiComboFlags_HeightLarge)) {
//                if (ImGui::Selectable("Constant")) { add_constant(); }
//                if (ImGui::Selectable("Slider")) { add_slider(); }
//                if (ImGui::Selectable("Function")) { add_function(); }
//                ImGui::EndCombo();
//            }
//
//            ImGui::Text("Constants:", "");
//            ImGui::Spacing();
//
//            ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, {5, 5});
//            if (ImGui::BeginTable("const_table",
//                                  1,
//                                  ImGuiTableFlags_BordersH | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY,
//                                  ImVec2{ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionMax().y * .3f})) {
//
//                int const_to_delete = -1;
//                for (int i = 0; i < constants.size(); ++i) {
//                    ImGui::PushID(i + 1);
//                    ImGui::TableNextRow();
//                    ImGui::TableSetColumnIndex(0);
//
//                    if (const_id != i) {
//                        ImGui::Text("%s=%s", constants[i].name.c_str(), constants[i].value.c_str());
//
//                        if (ImGui::BeginPopupContextItem("delete constant")) {
//                            if (ImGui::Button("Delete?")) { const_to_delete = i; }
//
//                            ImGui::EndPopup();
//                        }
//
//                    } else {
//                        if (ImGui::InputText("##new_const_name_val", &func_str, ImGuiInputTextFlags_EnterReturnsTrue)) {
//                            constants[i].name
//                                = std::string{func_str.begin(), func_str.begin() + func_str.find_first_of('=')};
//                            constants[i].value
//                                = std::string{func_str.begin() + func_str.find_first_of('=') + 1, func_str.end()};
//                            const_id = -1;
//                            make_program();
//                        }
//                    }
//
//                    if (ImGui::IsMouseDoubleClicked(0) && ImGui::IsItemHovered() && const_id == -1) {
//                        const_id = i;
//                        func_str = constants[i].name + "=" + constants[i].value;
//                    }
//
//                    ImGui::PopID();
//                }
//                if (const_to_delete > -1) { constants.erase(constants.begin() + const_to_delete); }
//
//                ImGui::EndTable();
//            }
//
//            ImGui::Spacing();
//            ImGui::Spacing();
//
//            ImGui::Text("Sliders:");
//            ImGui::Spacing();
//            if (ImGui::BeginTable("slider_table",
//                                  3,
//                                  ImGuiTableFlags_BordersH | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY,
//                                  ImVec2{ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionMax().y * .3f})) {
//
//                int slider_to_delete = -1;
//                for (int i = 0; i < sliders.size(); ++i) {
//                    ImGui::PushID(i + 1);
//                    auto &s = sliders[i];
//                    ImGui::TableNextRow();
//                    ImGui::TableSetColumnIndex(0);
//                    ImGui::SetNextItemWidth(10.f);
//                    ImGui::Text("From: ", "");
//                    ImGui::SameLine();
//                    ImGui::InputFloat("##from", &s.from);
//                    ImGui::TableSetColumnIndex(1);
//                    ImGui::SameLine();
//                    ImGui::Text("To: ", "");
//                    ImGui::SameLine();
//                    ImGui::InputFloat("##to", &s.to);
//                    ImGui::SameLine();
//                    ImGui::TableSetColumnIndex(2);
//
//                    if (slider_id != i) {
//                        if (ImGui::SliderFloat(s.name.c_str(), &s.value, s.from, s.to)) { make_program(); }
//                        if (ImGui::BeginPopupContextItem("delete_slider")) {
//                            if (ImGui::Button("Delete?")) { slider_to_delete = i; }
//                            ImGui::EndPopup();
//                        }
//                    } else {
//                        ImGui::SetKeyboardFocusHere(1);
//                        if (ImGui::InputText("##slider_new_name", &func_str, ImGuiInputTextFlags_EnterReturnsTrue)) {
//                            s.name    = func_str;
//                            slider_id = -1;
//                            make_program();
//                        }
//                    }
//
//                    if (ImGui::IsMouseDoubleClicked(0) && ImGui::IsItemHovered() && slider_id == -1) {
//                        slider_id = i;
//                        func_str  = s.name;
//                    }
//                    ImGui::PopID();
//                }
//
//                if (slider_to_delete > -1) { sliders.erase(sliders.begin() + slider_to_delete); }
//                ImGui::EndTable();
//            }
//
//            ImGui::Spacing();
//            ImGui::Spacing();
//            ImGui::Text("Functions:", "");
//            ImGui::Spacing();
//
//            if (ImGui::BeginTable("func_table",
//                                  1,
//                                  ImGuiTableFlags_BordersH | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY,
//                                  ImVec2{ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionMax().y * .3f})) {
//
//                int func_to_delete = -1;
//                for (int i = 0; i < functions.size(); ++i) {
//                    ImGui::PushID(i + 1);
//                    ImGui::TableNextRow();
//                    ImGui::TableSetColumnIndex(0);
//                    ImGui::BeginGroup();
//
//                    if (func_id != i) {
//                        ImGui::Text((functions[i].name + "(x,z)=").c_str());
//                        if (ImGui::BeginPopupContextItem("delete constant")) {
//                            if (ImGui::Button("Delete?")) { func_to_delete = i; }
//
//                            ImGui::EndPopup();
//                        }
//
//                    } else {
//                        if (ImGui::InputText("##new_func_name", &func_str, ImGuiInputTextFlags_EnterReturnsTrue)) {
//                            functions[i].name = func_str;
//                            func_id           = -1;
//                            make_program();
//                        }
//                    }
//
//                    if (ImGui::IsMouseDoubleClicked(0) && ImGui::IsItemHovered() && func_id == -1) {
//                        func_id  = i;
//                        func_str = functions[i].name;
//                    }
//
//                    ImGui::SameLine();
//                    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
//                    if (ImGui::InputText(("##" + functions[i].name).c_str(), &functions[i].equation)) {
//
//                        std::string func;
//                        func.append("float ");
//                        func.append(functions[i].name);
//                        func.append("(x, z){return ");
//                        func.append(functions[i].equation);
//                        func.append(";}");
//                        make_program();
//                    }
//                    ImGui::EndGroup();
//
//                    ImGui::PopID();
//                }
//                if (func_to_delete > -1) { functions.erase(functions.begin() + func_to_delete); }
//                ImGui::EndTable();
//            }
//        }
//        ImGui::PopStyleVar();
//        ImGui::EndChild();
//
//        ImGui::SetCursorPos(cpos);
//        if (ImGui::BeginChild("Errors", ImVec2(window_w * .75f, 0), true)) { ImGui::Text("a"); }
//        ImGui::EndChild();
//    }
//    ImGui::End();
//}
//
//void Graph3D::make_program() {
//    std::filesystem::path p{"shaders"};
//
//    std::ofstream vert_sh{p / "3dcalc.vert"};
//    std::ofstream frag_sh{p / "3dcalc.frag"};
//
//    vert_sh << "#version 460 core\n";
//    vert_sh << "#define SIDE_LEN 1000\n";
//    vert_sh << "precision highp float;\n";
//    vert_sh << "layout(location=0) in vec2 pos;\n";
//    vert_sh << "uniform mat4 model;\n";
//    vert_sh << "uniform mat4 view;\n";
//    vert_sh << "uniform mat4 projection;\n";
//    vert_sh << "uniform float TIME;\n";
//
//    for (auto f = constants.crbegin(); f != constants.crend(); f++) {
//        if (f->name == "TIME") { break; }
//
//        std::stringstream ss;
//        ss << "const float " << f->name << "=" << f->value << ";\n";
//        vert_sh << ss.str();
//    }
//    for (auto f = sliders.crbegin(); f != sliders.crend(); f++) {
//        if (f->name == "TIME") { break; }
//
//        std::stringstream ss;
//        ss << "const float " << f->name << "=" << f->value << ";\n";
//        vert_sh << ss.str();
//    }
//
//    for (auto f = functions.crbegin(); f != functions.crend(); f++) {
//        std::stringstream ss;
//        ss << "float " << f->name << "(float x,float z){return " << f->equation << ";}\n";
//        vert_sh << ss.str();
//    }
//
//    vert_sh << R"glsl(
//flat out int inst_id;
//out vec3 vpos;
//out vec3 vnormal;
//
//void main() {
//inst_id = int(gl_InstanceID);
//vec3 p = vec3(pos.x, 0., pos.y) * 0.001;
//
//float offsetX = float(gl_InstanceID % SIDE_LEN) * 0.001;
//float offsetZ = float(gl_InstanceID / SIDE_LEN) * 0.001;
//
//p += vec3(offsetX, 0., offsetZ);
//p.xz = p.xz * 3.14*2. - 3.14;
//
//p.y = f(p.x, p.z);
//
//vpos = p;
//
//vec3 T = vec3(p.x+0.001, f(p.x+0.001, p.z), p.z);
//vec3 B = vec3(p.x, f(p.x, p.z+0.001), p.z + 0.001);
//vnormal = normalize(cross(T - p, B - p));
//
//gl_Position = projection * view * model * vec4(p, 1.);})glsl";
//
//    frag_sh << "#version 460 core\n"
//               "out vec4 FRAG_COLOR;\n"
//               "in vec3 vnormal;\n"
//               "in vec3 vpos;\n"
//               "flat in int inst_id;\n"
//               "void main() {\n"
//               "	FRAG_COLOR = vec4(vnormal*0.5+.5, 1.f);\n"
//               "}";
//    vert_sh.flush();
//    frag_sh.flush();
//    vert_sh.close();
//    frag_sh.close();
//    try {
//        eng::ShaderProgram prog{"3dcalc"};
//        eng::Engine::instance().renderer_->get_program(default_program) = std::move(prog);
//    } catch (const std::exception &e) { std::cout << e.what(); }
//}
//
//void Graph3D::add_constant() { constants.emplace_back("x", "0.f"); }
//
//void Graph3D::add_function() {
//    char name = 'f';
//    for (auto f = functions.rbegin(); f != functions.rend(); f++) {
//        if (f->name.length() == 1) {
//            name = f->name[0];
//            break;
//        }
//    }
//    name++;
//    functions.emplace_back(std::string{name}, "");
//}
//
//void Graph3D::add_slider() { sliders.emplace_back("slider", 0.f); }
