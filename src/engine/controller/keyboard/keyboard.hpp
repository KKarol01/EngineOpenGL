#pragma once

#include <glm/glm.hpp>

#include "../controller.hpp"

namespace eng {
    class Keyboard : public Controller {
      public:
        Keyboard();

        glm::vec3 move_vec() const final;
        glm::vec2 look_vec() const final;
        bool key_pressed(unsigned key) const final;
        void _update() final;

      private:
        glm::vec2 cursor_prev;
        glm::vec2 look_vec_;
    };
} // namespace eng
