#pragma once

#include "Shader.h" // esse Shader tem o GLAD

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <GLFW/glfw3.h>

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
};

