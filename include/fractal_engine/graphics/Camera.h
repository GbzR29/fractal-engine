#pragma once

#include "fractal_engine/graphics/Shader.h"

#include <third_party/glm/glm.hpp>
#include <third_party/glm/gtc/matrix_transform.hpp>
#include <third_party/glm/gtc/type_ptr.hpp>

#include <third_party/GLFW/glfw3.h>

namespace fractal_engine::graphics {

// declarações (extern) — apenas declaração no header
extern glm::vec3 cameraPos;
extern glm::vec3 cameraFront;
extern glm::vec3 cameraUp;

extern bool firstMouse;
extern float yaw;
extern float pitch;
extern float lastX;
extern float lastY;
extern float fov;


class Camera {
public:
    glm::vec3 position{0.0f, 1.8f, 0.0f};
    float yaw   = -90.0f;
    float pitch = 0.0f;
    float fov   = 45.0f;

    glm::mat4 getView() const;
    glm::mat4 getProjection(int w, int h) const;

    void rotate(float dx, float dy);

    // Retorna o vetor unitário de direção frontal (para onde a câmera está olhando)
    glm::vec3 getFront() const;
};

} // namespace fractal_engine::graphics

