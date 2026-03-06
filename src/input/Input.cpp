#include "fractal_engine/input/Input.h"

namespace fractal_engine::input {

void Input::update(GLFWwindow* window) {

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }    

    // Movimentação básica
    forward = glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS;
    back    = glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS;
    left    = glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS;
    right   = glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS;

    // Pulo
    jump    = glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS;

    // Guardar estado anterior dos cliques ANTES de atualizar
    prevLeftClick = leftClick;
    prevRightClick = rightClick;

    // Atualizar estado atual dos cliques
    leftClick  = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
    rightClick = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;

    // Seleção de blocos com números
    key1Pressed = glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS;
    key2Pressed = glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS;
    key3Pressed = glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS;
}

void Input::onMouseMove(double xposIn, double yposIn) {
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    mouseDX += xpos - lastX; 
    mouseDY += lastY - ypos; 

    lastX = xpos;
    lastY = ypos;
}

} // namespace fractal_engine::input