#pragma once

#include "../../engine.hpp"
#include "../../wrappers/buffer/buffer.hpp"
#include "../../wrappers/shader/shader_dec.hpp"

struct RenderData {
//    GLVao vao;
    ShaderProgram *sh{nullptr};
    ShaderStorage sh_datas;
    std::unordered_map<std::uint32_t, std::uint32_t> textures;
};