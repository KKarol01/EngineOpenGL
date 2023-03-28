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

struct ShaderDataWrapper {
    using SupportedTypes = std::variant<std::reference_wrapper<int>,
                                        std::reference_wrapper<float>,
                                        std::reference_wrapper<glm::vec2>,
                                        std::reference_wrapper<glm::vec3>,
                                        std::reference_wrapper<glm::mat4>>;

    ShaderDataWrapper() = default;

    template <typename T> ShaderDataWrapper(T &&t) {
        if constexpr (std::is_pointer_v<T>) {
            data      = std::shared_ptr<void>{t, [](void *) {}};
            conv_func = [](void *ptr) { return SupportedTypes{*static_cast<T>(ptr)}; };
        } else if constexpr (std::is_lvalue_reference_v<decltype(t)>) {
            data      = std::shared_ptr<void>{new std::remove_cvref_t<T>{t}};
            conv_func = [](void *ptr) { return SupportedTypes{*static_cast<T *>(ptr)}; };
        } else if constexpr (std::is_rvalue_reference_v<decltype(t)>) {
            data      = std::shared_ptr<void>{new std::remove_cvref_t<T>{std::move(t)}};
            conv_func = [](void *ptr) { return SupportedTypes{*static_cast<T *>(ptr)}; };
        }
    }

    SupportedTypes operator()() { return conv_func(data.get()); };
    const SupportedTypes operator()() const { return conv_func(data.get()); };

    std::shared_ptr<void> data;
    SupportedTypes (*conv_func)(void *) = [](void *) {
        assert("type not initialised." && false);
        int x;
        return SupportedTypes{x};
    };
};

struct ShaderStorage {
    std::unordered_map<std::string, ShaderDataWrapper> data;

    ShaderDataWrapper &operator[](const std::string &name) { return data[name]; }
};

class ShaderProgram {
  public:
    ShaderProgram() = default;
    explicit ShaderProgram(const std::string &file_name);

    ShaderProgram(const ShaderProgram &) noexcept;
    ShaderProgram(ShaderProgram &&) noexcept;
    ShaderProgram &operator=(const ShaderProgram &) noexcept;
    ShaderProgram &operator=(ShaderProgram &&) noexcept;
    ~ShaderProgram();

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