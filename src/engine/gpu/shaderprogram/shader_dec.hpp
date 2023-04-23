#pragma once

#include <type_traits>
#include <string>
#include <string_view>
#include <variant>
#include <vector>
#include <concepts>
#include <memory>
#include <functional>
#include <utility>

#include <glm/glm.hpp>

#include "../../renderer/typedefs.hpp"

namespace eng {
    class ShaderProgram :public RendererResource{
      public:
        ShaderProgram() = default;
        explicit ShaderProgram(const std::string &file_name);

        ShaderProgram(const ShaderProgram &) noexcept;
        ShaderProgram(ShaderProgram &&) noexcept;
        ShaderProgram &operator=(const ShaderProgram &) noexcept;
        ShaderProgram &operator=(ShaderProgram &&) noexcept;
        ~ShaderProgram();

        bool operator==(const ShaderProgram& other) const {
            return file_name == other.file_name;
        }

      public:
        template <typename T>
        requires std::is_integral_v<T>
        void set(std::string_view name, const T &t);
        template <typename T>
        requires std::is_floating_point_v<T>
        void set(std::string_view name, const T &t);
        template <typename T>
        requires std::is_same_v<T, glm::vec2>
        void set(std::string_view name, const T &t);
        template <typename T>
        requires std::is_same_v<T, glm::vec3>
        void set(std::string_view name, const T &t);
        template <typename T>
        requires std::is_same_v<T, glm::vec4>
        void set(std::string_view name, const T &t);
        template <typename T>
        requires std::is_same_v<T, glm::mat4>
        void set(std::string_view name, const T &t);

      public:
        void use();
        void recompile();

        auto get_handle() const { return program_id; }

      private:
        unsigned program_id{0u};
        std::string file_name;
        static inline const std::string SHADERS_DIR = "shaders/";
        enum class SHADER_TYPE : unsigned {
            VERTEX   = 1 << 0,
            FRAGMENT = 1 << 1,
            COMPUTE  = 1 << 2,
            TESS_C   = 1 << 3,
            TESS_E   = 1 << 4,
        };
    };
} // namespace eng
