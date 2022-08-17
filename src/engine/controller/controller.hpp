#pragma once

#include <map>

#include <glm/glm.hpp>
#include "../signal/signal.hpp"

struct GLFWwindow;

struct ControllerInputInfo {
    GLFWwindow *window{nullptr};
    int key;
    int scancode;
    int actions;
    int mods;

    constexpr ControllerInputInfo(GLFWwindow *w, int k, int s, int a, int m)
        : window{w}, key{k}, scancode{s}, actions{a}, mods{m} {}
    constexpr inline bool action(int req) const { return (actions & req); }
    constexpr inline bool mod(int req) const { return (mods & req); }
};

class Controller {
    std::map<uint32_t, Signal<ControllerInputInfo>> key_down;

  public:
    Controller();

  public:
    virtual glm::vec2 look_vec() const       = 0;
    virtual glm::vec3 move_vec() const       = 0;
    virtual bool key_pressed(unsigned) const = 0;

    virtual void update() = 0;

  protected:
    virtual void _key_callback(GLFWwindow *, int, int, int, int);

    friend void glfw_callback_handler(GLFWwindow *, int, int, int, int);
};