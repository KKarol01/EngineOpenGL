#pragma once

#include "../controller.hpp"

class Keyboard : public Controller {
  private:
    glm::vec2 cursor_prev;
    glm::vec2 look_vec_;

  public:
    Keyboard();

    glm::vec3 move_vec() const final;
    glm::vec2 look_vec() const final;
    bool key_pressed(unsigned key) const final;
    void update() final;
};