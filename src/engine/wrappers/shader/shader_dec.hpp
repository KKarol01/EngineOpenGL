#pragma once

#include <type_traits>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

#include <glm/glm.hpp>

struct SHADERDATA {
    std::string name;
    std::variant<int, float, glm::vec2, glm::vec3, glm::mat4> value;
};

class Shader {
  private:
    unsigned program_id{0u};
    std::string file_name;
    static inline const std::string SHADERS_DIR = "shaders/";
    enum class SHADER_TYPE: unsigned {
        VERTEX   = 1 << 0,
        FRAGMENT = 1 << 1,
        COMPUTE  = 1 << 2,
        TESS_C   = 1 << 3,
        TESS_E   = 1 << 4,
    };

  public:
    Shader(const std::string &file_name);

    Shader() = default;
    Shader(const Shader&)               noexcept;
    Shader(Shader&&)                    noexcept;
    Shader& operator=(const Shader&)    noexcept;
    Shader& operator=(Shader&&)         noexcept;
    ~Shader();

  public:
      template <typename T> requires std::is_integral_v<T>			void set(std::string_view name, const T& t);
	  template <typename T> requires std::is_floating_point_v<T>	void set(std::string_view name, const T& t);
	  template <typename T> requires std::is_same_v<T, glm::vec2>	void set(std::string_view name, const T& t);
	  template <typename T> requires std::is_same_v<T, glm::vec3>	void set(std::string_view name, const T& t);
	  template <typename T> requires std::is_same_v<T, glm::vec4>	void set(std::string_view name, const T& t);
	  template <typename T> requires std::is_same_v<T, glm::mat4>	void set(std::string_view name, const T& t);

  public:
    void use();
    void feed_uniforms(const std::vector<SHADERDATA>& data);
    void recompile();

    auto get_program() const { return program_id; }
};