#pragma once
#include <third_party/GLFW/glfw3.h>

namespace fractal_engine::input {

class Input {
public:
    void update(GLFWwindow* window);
    void onMouseMove(double xposIn, double yposIn);

    // Movimento e ações
    bool forward = false;
    bool back    = false;
    bool left    = false;
    bool right   = false;
    bool jump    = false;

    // Cliques do mouse (estado atual)
    bool leftClick  = false;   // Quebrar bloco
    bool rightClick = false;   // Colocar bloco

    // Detecção de transição (apenas quando muda de false para true)
    bool leftClickPressed()  const { return leftClick && !prevLeftClick; }
    bool rightClickPressed() const { return rightClick && !prevRightClick; }

    // Seleção de blocos (números 1, 2, 3)
    bool key1Pressed = false;  // BLOCK_STONE
    bool key2Pressed = false;  // BLOCK_GRASS
    bool key3Pressed = false;  // BLOCK_DIRT

    // Movimento do mouse
    float mouseDX = 0.0f;
    float mouseDY = 0.0f;

private:
    float lastX = 0.0f;
    float lastY = 0.0f;
    bool firstMouse = true;

    // Estados anteriores para detectar transição
    bool prevLeftClick = false;
    bool prevRightClick = false;
};

} // namespace fractal_engine::input