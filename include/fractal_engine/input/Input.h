#pragma once
#include <third_party/GLFW/glfw3.h>

namespace fractal_engine::input {

class Input {
public:
    void update(GLFWwindow* window);
    void onMouseMove(double xposIn, double yposIn);
    void onScroll(double xoffset, double yoffset);  // ← novo

    // Movimento e ações
    bool forward = false;
    bool back    = false;
    bool left    = false;
    bool right   = false;
    bool jump    = false;

    // Cliques do mouse
    bool leftClick  = false;
    bool rightClick = false;

    bool leftClickPressed()  const { return leftClick  && !prevLeftClick;  }
    bool rightClickPressed() const { return rightClick && !prevRightClick; }

    // Teclas numéricas (slot da hotbar)
    bool key1Pressed = false;
    bool key2Pressed = false;
    bool key3Pressed = false;
    bool key4Pressed = false;
    bool key5Pressed = false;
    bool key6Pressed = false;
    bool key7Pressed = false;
    bool key8Pressed = false;
    bool key9Pressed = false;
    bool key0Pressed = false;

    // Scroll — acumulado pelo callback, lido pela Hotbar
    int scrollVal = 0;

    // Movimento do mouse
    float mouseDX = 0.0f;
    float mouseDY = 0.0f;

private:
    float lastX      = 0.0f;
    float lastY      = 0.0f;
    bool  firstMouse = true;

    bool prevLeftClick  = false;
    bool prevRightClick = false;
};

} // namespace fractal_engine::input