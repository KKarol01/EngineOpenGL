#include "camera.hpp"
#include "../engine.hpp"
#include "../controller/controller.hpp"
#include "../wrappers/window/window.hpp"
#include <glm/gtc/quaternion.hpp>
#include <iostream>

Camera::Camera() { update_projection(); }

void Camera::update() {
    const auto &controller = *eng::Engine::instance().controller();
    yaw_pitch += controller.look_vec();
    yaw_pitch.y = glm::clamp(yaw_pitch.y, -89.f, 89.f);

    glm::quat rotation;
    rotation = glm::angleAxis(glm::radians(yaw_pitch.x), up);
    rotation *= glm::angleAxis(glm::radians(yaw_pitch.y), right);

    look_forward = rotation * glm::vec3{0.f, 0.f, -1.f};
    look_right   = glm::cross(look_forward, up);

    auto input_vec = controller.move_vec() * movement_speed * (float)eng::Engine::instance().deltatime();
    m_position += glm::normalize(look_right * plane_constraint) * input_vec.x;
    m_position.y += (look_up * input_vec.y).y;
    m_position += glm::normalize(look_forward * plane_constraint) * input_vec.z;

    look_forward = glm::normalize(look_forward);
    look_right   = glm::normalize(look_right);
}

glm::mat4 Camera::view_matrix() const { return glm::lookAt(m_position, m_position + look_forward, up); }

void Camera::update_projection() {
    projection = glm::perspective(glm::radians(lens.fovydeg), eng::Engine::instance().window()->aspect(), lens.near, lens.far);
}
