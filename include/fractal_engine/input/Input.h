#pragma once
#include <third_party/GLFW/glfw3.h>

namespace fractal_engine::input {

class Input {
public:
    void update(GLFWwindow* window);
    void onMouseMove(double xposIn, double yposIn);

    bool forward = false;
    bool back    = false;
    bool left    = false;
    bool right   = false;
    bool jump    = false; // Nova variável para o pulo

    float mouseDX = 0.0f;
    float mouseDY = 0.0f;

private:
    float lastX = 0.0f;
    float lastY = 0.0f;
    bool firstMouse = true;
};

} // namespace fractal_engine::input