#pragma once

#include "../engine.hpp"

struct RenderData {
    VAO vao;
    Shader *sh{nullptr};
    std::vector<SHADERDATA> sh_datas;
};