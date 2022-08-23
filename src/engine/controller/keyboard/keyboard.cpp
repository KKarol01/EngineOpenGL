#include "keyboard.hpp"

#include "../../engine.hpp"
#include "../../wrappers/window/window.hpp"

#include <GLFW/glfw3.h>

Keyboard::Keyboard() : Controller() {
    auto window = Engine::instance().window()->glfwptr();
    double cx, cy;
    glfwGetCursorPos(window, &cx, &cy);
    glfwSetCursorPos(window, cx, cx);
    cursor_prev = {cx, cy};
}

glm::vec3 Keyboard::move_vec() const {
    auto window = Engine::instance().window()->glfwptr();

    const auto W      = glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS;
    const auto S      = glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS;
    const auto A      = glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS;
    const auto D      = glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS;
    const auto SPACE  = glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS;
    const auto LSHIFT = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS;

    return {A ? -1.f : D ? 1.f : 0.f, SPACE ? 1.f : LSHIFT ? -1.f : 0.f, W ? 1.f : S ? -1.f : 0.f};
}

glm::vec2 Keyboard::look_vec() const { return look_vec_; }

bool Keyboard::key_pressed(unsigned key) const { return glfwGetKey(Engine::instance().window()->glfwptr(), key) == GLFW_PRESS; }

void Keyboard::update() {
    double cx, cy;
    glfwGetCursorPos(Engine::instance().window()->glfwptr(), &cx, &cy);
    glm::vec2 cursor{cx, cy};

    look_vec_   = -(cursor - cursor_prev);
    cursor_prev = cursor;
}
