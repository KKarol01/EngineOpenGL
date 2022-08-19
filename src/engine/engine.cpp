#include "engine.hpp"

#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

void Engine::initialize(Window &&window) {
    Engine::window_         = std::make_unique<Window>(std::move(window));
    Engine::controller_     = std::make_unique<Keyboard>();
    Engine::shader_manager_ = std::make_unique<ShaderManager>();
    Engine::gui_            = std::make_unique<GUI>();
    Engine::ecs_            = std::make_unique<ECS>();
    Engine::renderer_       = std::make_unique<Renderer>();

    time = dt = glfwGetTime();
}

void Engine::update() {
    dt   = glfwGetTime() - time;
    time = glfwGetTime();

    controller_->update();
}

void Engine::render_frame() {
    glClearColor(0.1f, 0.2f, 0.4f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    renderer_->render_frame();
    gui_->draw();
    window_->swap_buffers();
}

void Engine::terminate() {
    window_->close();
    glfwTerminate();
}
