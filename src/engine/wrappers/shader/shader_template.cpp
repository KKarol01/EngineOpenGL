#include "shader_dec.hpp"

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

static unsigned get_uniform_location(std::string_view name, unsigned program) {
    return glGetUniformLocation(program, name.data());
}

template <typename T> requires std::is_integral_v<T>		void Shader::set(std::string_view name, const T &t) {
    glUniform1iv(get_uniform_location(name, program_id), 1, &t);
}

template <typename T> requires std::is_floating_point_v<T>	void Shader::set(std::string_view name, const T &t) {
    glUniform1fv(get_uniform_location(name, program_id), 1, &t);
}

template<typename T> requires std::is_same_v<T, glm::vec2> void Shader::set(std::string_view name, const T &t) {
    glUniform2fv(get_uniform_location(name, program_id), 1, glm::value_ptr(t));
}

template <typename T> requires std::is_same_v<T, glm::vec3> void Shader::set(std::string_view name, const T &t) {
    glUniform3fv(get_uniform_location(name, program_id), 1, glm::value_ptr(t));
}

template <typename T> requires std::is_same_v<T, glm::vec4> void Shader::set(std::string_view name, const T &t) {
    glUniform4fv(get_uniform_location(name, program_id), 1, glm::value_ptr(t));
}

template <typename T> requires std::is_same_v<T, glm::mat4>	void Shader::set(std::string_view name, const T &t) {
    glUniformMatrix4fv(get_uniform_location(name, program_id), 1, GL_FALSE, glm::value_ptr(t));
}