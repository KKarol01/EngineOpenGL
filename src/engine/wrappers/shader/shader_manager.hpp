#pragma once

#include "shader.hpp"
#include <map>
#include <string>
#include <optional>

class ShaderManager {
    std::map<std::string, Shader> shaders;

  public:
    Shader *get_shader(const std::string &name);
    std::optional<const Shader *>get_shader(const std::string &name) const;
};