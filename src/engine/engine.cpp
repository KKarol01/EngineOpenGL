#include "engine.hpp"

#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "camera/camera.hpp"
#include "subsystems/renderer/renderer.hpp"
#include "subsystems/gui/gui.hpp"
#include "controller/keyboard/keyboard.hpp"
#include "signal/signal.hpp"
#include "subsystems/ecs/ecs.d.hpp"
#include "wrappers/include_all.hpp"

Engine::Engine(Window &&window) noexcept { *this = std::move(window); }

Engine &Engine::operator=(Window &&window) noexcept {
    Engine::window_         = std::make_unique<Window>(std::move(window));
    Engine::controller_     = std::make_unique<Keyboard>();
    Engine::shader_manager_ = std::make_unique<ShaderManager>();
    Engine::ecs_            = std::make_unique<ECS>();
    Engine::renderer_       = std::make_unique<Renderer>();
    Engine::gui_            = std::make_unique<GUI>();

    time = dt = glfwGetTime();
    return *this;
}

Engine::~Engine() {
    window_->close();
    glfwTerminate();
}

void Engine::update() {
    dt   = glfwGetTime() - time;
    time = glfwGetTime();

    controller_->update();

    glClearColor(0.3f, 0.8f, 0.21f, 1.f);
    window_->clear_framebuffer();
    renderer_->render_frame();
    gui_->draw();
    window_->swap_buffers();
}

void Engine::initialise(Window &&w) { _instance = std::move(w); }

Engine Engine::_instance = Engine{};