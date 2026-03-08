#include "fractal_engine/input/Input.h"

namespace fractal_engine::input {

void Input::update(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    forward = glfwGetKey(window, GLFW_KEY_W)     == GLFW_PRESS;
    back    = glfwGetKey(window, GLFW_KEY_S)     == GLFW_PRESS;
    left    = glfwGetKey(window, GLFW_KEY_A)     == GLFW_PRESS;
    right   = glfwGetKey(window, GLFW_KEY_D)     == GLFW_PRESS;
    jump    = glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS;

    prevLeftClick  = leftClick;
    prevRightClick = rightClick;
    leftClick  = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT)  == GLFW_PRESS;
    rightClick = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;

    key1Pressed = glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS;
    key2Pressed = glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS;
    key3Pressed = glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS;
    key4Pressed = glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS;
    key5Pressed = glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS;
    key6Pressed = glfwGetKey(window, GLFW_KEY_6) == GLFW_PRESS;
    key7Pressed = glfwGetKey(window, GLFW_KEY_7) == GLFW_PRESS;
    key8Pressed = glfwGetKey(window, GLFW_KEY_8) == GLFW_PRESS;
    key9Pressed = glfwGetKey(window, GLFW_KEY_9) == GLFW_PRESS;
    key0Pressed = glfwGetKey(window, GLFW_KEY_0) == GLFW_PRESS;
}

void Input::onMouseMove(double xposIn, double yposIn)
{
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

// ── Novo ──────────────────────────────────────────────────────────────────────
// yoffset: +1 = scroll para cima, -1 = scroll para baixo
// scrollVal é acumulado — a Hotbar lê a diferença entre frames
void Input::onScroll(double /*xoffset*/, double yoffset)
{
    scrollVal += static_cast<int>(yoffset);
}

} // namespace fractal_engine::input