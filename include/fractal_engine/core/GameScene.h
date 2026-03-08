#pragma once
#include "fractal_engine/graphics/Shader.h"
#include "fractal_engine/world/World.h"
#include "fractal_engine/scene/Player.h"
#include "fractal_engine/scene/SkySystem.h"
#include "fractal_engine/input/Input.h"
#include "fractal_engine/core/Window.h"
#include "fractal_engine/world/BlockOutline.h"

using namespace fractal_engine::graphics;
using namespace fractal_engine::world;
using namespace fractal_engine::scene;
using namespace fractal_engine::input;
using namespace fractal_engine::core;

class GameScene {
public:
    GameScene(Shader& worldShader, Shader& uiShader, Shader& skyShader,
              int width, int height);

    void init();
    void update(float dt, Input& input, Window& window);
    void render(Input& input, Window& window);

private:
    Shader& worldShader;
    Shader& uiShader;
    Shader& skyShader;    // ← novo
    int     screenW, screenH;

    World     world;
    Player    player;
    SkySystem sky;
    fractal_engine::renderer::BlockOutline blockOutline;

    GLuint crosshairVAO = 0, crosshairVBO = 0;
    void initCrosshair();
    void drawCrosshair();
};