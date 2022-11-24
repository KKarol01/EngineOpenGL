#pragma once

#include <glm/glm.hpp>

class Camera {
  public:
    Camera();

  public:
    static inline const glm::vec3 forward{0.f, 0.f, -1.f}, right{1.f, 0.f, 0.f}, up{0.f, 1.f, 0.f};
    static inline float movement_speed{.5f};

    struct LensSettings {
        float fovydeg{90.f}, near{0.01f}, far{100.f};
    } lens;

  private:
    glm::mat4 projection;
    glm::vec3 m_position{0.f};
    glm::vec2 yaw_pitch{0.f};
    glm::vec3 plane_constraint{1.f, 0.f, 1.f};
    glm::vec3 look_forward{forward}, look_right{right}, look_up{up};

  public:
    void update();

    inline void adjust_lens(const LensSettings &ls) {
        lens = ls;
        update_projection();
    };
    void update_projection();

    glm::mat4 view_matrix() const;
    glm::mat4 perspective_matrix() const { return projection; };
    glm::vec3 forward_vec() const { return glm::normalize(m_position + look_forward); }
    glm::vec3 right_vec() const { return glm::normalize(glm::cross(forward_vec(), up)); }
    glm::vec3 position() const { return m_position; }
    glm::vec3 &position() { return m_position; }
};