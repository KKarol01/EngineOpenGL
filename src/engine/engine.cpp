#include "engine.hpp"

#include <iostream>
#include <cstdint>
#include <string_view>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "controller/controller.hpp"
#include "controller/keyboard/keyboard.hpp"

eng::Engine::~Engine() {
    _window->close();
    glfwTerminate();
}

void eng::Engine::_update() {
    glfwPollEvents();
    _controller->_update();
    _camera->_update();

    _window->clear_framebuffer();
    //    renderer_->render_frame();
    _gui->draw();
    _window->swap_buffers();
}

void eng::Engine::start() {
    while (_window->should_close() == false) { _update(); }
}

void eng::Engine::initialise(std::string_view window_name, uint32_t size_x, uint32_t size_y) {
    eng::Engine::_instance = std::make_unique<eng::Engine>();
    auto this_             = eng::Engine::_instance.get();

    this_->_window      = std::make_unique<Window>(window_name, size_x, size_y);
    this_->_camera      = std::make_unique<Camera>();
    this_->_controller  = std::make_unique<Keyboard>();
    this_->_gpu_res_mgr = std::make_unique<GpuResMgr>();
    this_->_renderer    = std::make_unique<Renderer>();
    this_->_gui         = std::make_unique<GUI>();
}