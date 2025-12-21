#include "../headers/Camera.h"
#include <algorithm>

glm::mat4 Camera::getView() const {

    glm::vec3 front{
        cos(glm::radians(yaw)) * cos(glm::radians(pitch)),
        sin(glm::radians(pitch)),
        sin(glm::radians(yaw)) * cos(glm::radians(pitch))
    };

    return glm::lookAt(
        position,
        position + glm::normalize(front),
        glm::vec3(0.0f, 1.0f, 0.0f)
    );
}

glm::mat4 Camera::getProjection(int width, int height) const {
    return glm::perspective(
        glm::radians(fov),
        (float)width / (float)height,
        0.01f,
        500.0f
    );
}

void Camera::rotate(float dx, float dy) {

    const float sensitivity = 0.1f;
    yaw   += dx * sensitivity;
    pitch += dy * sensitivity;

    pitch = std::clamp(pitch, -89.0f, 89.0f);
}
