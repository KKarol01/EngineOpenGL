#pragma once

#include <cstdint>
#include "../engine/types/types.hpp"
#include "../engine/renderer/renderer.hpp"

class Graph3D {
  public:
    Graph3D();

    void render();

    uint32_t fbo, rbo, coltex, depthtex;
    eng::SharedResource<eng::ShaderProgram> default_program;
    eng::SharedResource<eng::Pipeline> pipeline;

  private:
    struct Function {
        std::string name;
        std::string equation;
    };
    struct Constant {
        std::string name;
        std::string value;
    };
    struct Slider {
        std::string name;
        float value=0;
        float from=-1, to=1;
    };

    void draw_gui();
    void make_program();

    void add_constant();
    void add_function();
    void add_slider();

    std::vector<Function> functions{{"f", "sin(x)"}};
    std::vector<Constant> constants{{"TIME", "0.f"}};
    std::vector<Slider> sliders{};

    int func_id = -1, const_id=-1, slider_id=-1;
    std::string func_str;

    bool wireframe = false;
};