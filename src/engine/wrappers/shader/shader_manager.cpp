#include "shader_manager.hpp"

Shader *ShaderManager::get_shader(const std::string &name) {
    shaders[name] = Shader{name};
    return &shaders[name];
}

std::optional<const Shader *> ShaderManager::get_shader(const std::string &name) const {
    if (shaders.contains(name)) return std::make_optional(&shaders.at(name));
    return std::nullopt;
}
