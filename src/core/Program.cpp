#define STB_IMAGE_IMPLEMENTATION

#include "Program.h"
#include "GameScene.h"
#include "fractal_engine/graphics/Shader.h"
#include "fractal_engine/input/Input.h"
#include "fractal_engine/core/Window.h"
#include <third_party/glm/glm.hpp>
#include <iostream>

using namespace fractal_engine::graphics;
using namespace fractal_engine::input;
using namespace fractal_engine::core;

// ─────────────────────────────────────────────────────────────────────────────
// Constantes de tela
// ─────────────────────────────────────────────────────────────────────────────
static constexpr int   SCREEN_W   = 1400;
static constexpr int   SCREEN_H   = 900;
static constexpr float MAX_DT     = 0.05f;  // clamp: nunca simula mais de 50ms/frame

// ─────────────────────────────────────────────────────────────────────────────
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

    // ── Shaders ──────────────────────────────────────────────────────────────
    Shader worldShader("shaders/vertex_shader.vert", "shaders/fragment_shader.frag");
    Shader uiShader   ("shaders/ui_vShader.vert",    "shaders/ui_fShader.frag");

    // ── Cenas ────────────────────────────────────────────────────────────────
    GameScene gameScene(worldShader, uiShader, SCREEN_W, SCREEN_H);

    // init() gera o mundo completo ANTES de iniciar o timer
    // → elimina o deltaTime gigante do primeiro frame
    gameScene.init();

    // ─────────────────────────────────────────────────────────────────────────
    // FIX: lastFrame é resetado AQUI, depois de toda a inicialização pesada.
    //
    // Antes: lastFrame = 0.0f (declarado no topo do arquivo). O generateWorld
    // pode levar 2-10 segundos gerando 289 chunks. O primeiro deltaTime era
    // glfwGetTime() - 0 = esses mesmos segundos. Com gravity=-9.8 e dt=5s,
    // velocity.y = -49, position.y -= 245 num único frame → player sai dos
    // limites do chunk → isAirWorld retorna true → sem colisão → void infinito.
    //
    // Agora: timer começa só após init(), então o primeiro dt é ~0ms.
    // ─────────────────────────────────────────────────────────────────────────
    float lastFrame = (float)glfwGetTime();

    // ── Loop principal ────────────────────────────────────────────────────────
    while (!window.shouldClose())
    {
        float currentFrame = (float)glfwGetTime();
        float dt = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Clamp de segurança: spikes de CPU/GPU não causam tunneling de física
        if (dt > MAX_DT) dt = MAX_DT;

        Input& input = window.getInput();
        input.update(window.getNativeWindow());

        gameScene.update(dt, input, window);
        gameScene.render(input, window);

        window.swapBuffers();
        window.pollEvents();
    }
}