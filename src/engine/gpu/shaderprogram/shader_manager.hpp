#pragma once

#include "shader.hpp"
#include <map>
#include <string>
#include <optional>

class ShaderManager {
    std::map<std::string, ShaderProgram> shaders;

  public:
    ShaderProgram *get_shader(const std::string &name);
    std::optional<const ShaderProgram *>get_shader(const std::string &name) const;
};