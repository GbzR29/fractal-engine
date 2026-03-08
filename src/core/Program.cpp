#define STB_IMAGE_IMPLEMENTATION
#include "fractal_engine/core/Program.h"
#include "fractal_engine/core/GameScene.h"
#include "fractal_engine/graphics/Shader.h"
#include "fractal_engine/input/Input.h"
#include "fractal_engine/core/Window.h"
#include <third_party/glm/glm.hpp>
#include <iostream>

using namespace fractal_engine::graphics;
using namespace fractal_engine::input;
using namespace fractal_engine::core;

static constexpr int   SCREEN_W = 1400;
static constexpr int   SCREEN_H = 900;
static constexpr float MAX_DT   = 0.05f;

void Init_Program()
{
    Window window(SCREEN_W, SCREEN_H, "Fractal Engine");
    if (!window.init()) {
        std::cerr << "Failed to initialize window.\n";
        return;
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    // ── Shaders ───────────────────────────────────────────────────────────
    Shader worldShader   ("shaders/glsl/vertex/chunk.vert",    "shaders/glsl/fragment/chunk.frag");
    Shader uiShader      ("shaders/ui_vShader.vert",           "shaders/ui_fShader.frag");
    Shader skyShader     ("shaders/glsl/vertex/sky.vert",      "shaders/glsl/fragment/sky.frag");
    Shader hudSlotShader ("shaders/glsl/hud/hud_slot.vert",    "shaders/glsl/hud/hud_slot.frag");
    Shader hudIconShader ("shaders/glsl/hud/hud_icon.vert",    "shaders/glsl/hud/hud_icon.frag");

    // ── Cena ──────────────────────────────────────────────────────────────
    GameScene gameScene(worldShader, uiShader, skyShader,
                        hudSlotShader, hudIconShader,
                        SCREEN_W, SCREEN_H);
    gameScene.init();

    float lastFrame = (float)glfwGetTime();
    while (!window.shouldClose())
    {
        float currentFrame = (float)glfwGetTime();
        float dt = currentFrame - lastFrame;
        lastFrame = currentFrame;
        if (dt > MAX_DT) dt = MAX_DT;

        Input& input = window.getInput();
        input.update(window.getNativeWindow());

        gameScene.update(dt, input, window);
        gameScene.render(input, window);

        window.swapBuffers();
        window.pollEvents();
    }
}