#pragma once

#include <cstdint>
#include "../engine/types/types.hpp"
#include "../engine/renderer/renderer.hpp"

class Graph3D {
  public:
    Graph3D();

    void render();

    uint32_t fbo, rbo, coltex, depthtex;
    eng::ProgramID default_program;
  private:
      struct Function {
        std::string name;
        std::string equation;
      };
      std::vector<Function> functions{{"f", "sin(x)"}};

      void draw_gui();
      void make_program();
};