#pragma once

#include "../../engine.hpp"
#include "../../wrappers/buffer/buffer.hpp"
#include "../../wrappers/shader/shader_dec.hpp"

struct RenderData {
    VAO vao;
    Shader *sh{nullptr};
    std::vector<SHADERDATA> sh_datas;
};