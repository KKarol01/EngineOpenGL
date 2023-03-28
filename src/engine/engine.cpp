#include "engine.hpp"

#include <iostream>
#include <cstdint>
#include <string_view>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "controller/controller.hpp"
#include "controller/keyboard/keyboard.hpp"

eng::Engine::~Engine() {
    window_->close();
    glfwTerminate();
}

void eng::Engine::update() {
    dt   = glfwGetTime() - time;
    time = glfwGetTime();

    controller_->update();

    glClearColor(0.3f, 0.8f, 0.21f, 1.f);
    window_->clear_framebuffer();
    //    renderer_->render_frame();
    gui_->draw();
    window_->swap_buffers();
}

void eng::Engine::initialise(std::string_view window_name, uint32_t size_x, uint32_t size_y) {
    eng::Engine::_instance = std::make_unique<eng::Engine>();
    auto this_             = eng::Engine::_instance.get();

    this_->window_         = std::make_unique<Window>(window_name, size_x, size_y);
    this_->controller_     = std::make_unique<Keyboard>();
    this_->shader_manager_ = std::make_unique<ShaderManager>();
    //this_->ecs_            = std::make_unique<ECS>();
    this_->renderer_       = std::make_unique<Renderer>();
    this_->gui_            = std::make_unique<GUI>();
    this_->time = this_->dt = glfwGetTime();
}