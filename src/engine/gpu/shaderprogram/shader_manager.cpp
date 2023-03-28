#include "shader_manager.hpp"

namespace eng {
    ShaderProgram *ShaderManager::get_shader(const std::string &name) {
        shaders[name] = ShaderProgram{name};
        return &shaders[name];
    }

    std::optional<const ShaderProgram *> ShaderManager::get_shader(const std::string &name) const {
        if (shaders.contains(name)) return std::make_optional(&shaders.at(name));
        return std::nullopt;
    }
} // namespace eng