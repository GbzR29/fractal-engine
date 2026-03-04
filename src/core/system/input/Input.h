#pragma once
#include <GLFW/glfw3.h>

class Input {
public:
    void update(GLFWwindow* window);
    void onMouseMove(double xposIn, double yposIn);

    bool forward  = false;
    bool back     = false;
    bool left     = false;
    bool right    = false;

    float mouseDX = 0.0f;
    float mouseDY = 0.0f;

private:
    float lastX = 0.0f;
    float lastY = 0.0f;
    bool firstMouse = true;
};
