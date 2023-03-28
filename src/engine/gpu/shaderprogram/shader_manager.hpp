#pragma once

#include <map>
#include <string>
#include <optional>

#include "shader.hpp"

namespace eng {
    class ShaderManager {
      public:
        ShaderProgram *get_shader(const std::string &name);
        std::optional<const ShaderProgram *> get_shader(const std::string &name) const;

      private:
        std::map<std::string, ShaderProgram> shaders;
    };
} // namespace eng