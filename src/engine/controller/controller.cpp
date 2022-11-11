#include "controller.hpp"

#include "../engine.hpp"
#include "../wrappers/window/window.hpp"

#include <iostream>
#include <GLFW/glfw3.h>

static void glfw_callback_handler(GLFWwindow *w, int k, int s, int a, int m) {
    eng::Engine::instance().controller()->_key_callback(w, k, s, a, m);
}

Controller::Controller() { glfwSetKeyCallback(eng::Engine::instance().window()->glfwptr(), (GLFWkeyfun)glfw_callback_handler); }

void Controller::_key_callback(GLFWwindow *w, int k, int s, int a, int m) {
    if (auto sig = key_down.find(k); sig != key_down.end()) { sig->second.emit(ControllerInputInfo{w, k, s, a, m}); }

    printf("%i %i %i %i\n", k, s, a, m);
}
