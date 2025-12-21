#include "../headers/Input.h"
#include <iostream>

void Input::update(GLFWwindow* window) {

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS){

        glfwSetWindowShouldClose(window, true);
    }    

    forward = glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS;
    back    = glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS;
    left    = glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS;
    right   = glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS;
}

void Input::onMouseMove(double xposIn, double yposIn) {

    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    mouseDX = xpos - lastX;
    mouseDY = lastY - ypos; // inverted because Y goes from bottom to top.
    lastX = xpos;
    lastY = ypos;
}
