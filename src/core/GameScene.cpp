#include "fractal_engine/core/GameScene.h"
#include "fractal_engine/world/BlockRegistry.h"
#include <iostream>

GameScene::GameScene(Shader& worldShader, Shader& uiShader,
                     Shader& skyShader,   Shader& hudSlotShader,
                     Shader& hudIconShader,
                     int width, int height)
    : worldShader   (worldShader)
    , uiShader      (uiShader)
    , skyShader     (skyShader)
    , hudSlotShader (hudSlotShader)
    , hudIconShader (hudIconShader)
    , screenW(width)
    , screenH(height)
{}

void GameScene::init() {
    BlockRegistry::init();

    // ── Player ────────────────────────────────────────────────────────────
    player.init();  // init da hotbar com GameMode::Creative por padrão

    // Para mudar para survival:
    // player.hotbar.setGameMode(GameMode::Survival);

    // ── Sky ───────────────────────────────────────────────────────────────
    SkyConfig skyCfg;
    skyCfg.dayDurationSeconds = 500.0f;
    skyCfg.ambientMin         = 0.08f;
    skyCfg.paused             = false;
    sky.init(skyShader, skyCfg);
    sky.setTimeOfDay(0.5f);

    // ── Mundo ─────────────────────────────────────────────────────────────
    world.generateWorld(8, 8, worldShader);
    player.initializeAtTerrainHeight(world, 0.0f, 0.0f);

    // ── HUD ───────────────────────────────────────────────────────────────
    hud.init(hudSlotShader, hudIconShader);

    // ── Outros ────────────────────────────────────────────────────────────
    blockOutline.init();
    initCrosshair();
}

void GameScene::update(float dt, Input& input, Window& window) {
    sky.update(dt);
    player.update(dt, input, world);
}

void GameScene::render(Input& input, Window& window) {
    glm::vec3 cc = sky.getClearColor();
    glClearColor(cc.r, cc.g, cc.b, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 view       = player.getView();
    glm::mat4 projection = player.getProjection(screenW, screenH);

    // ── Skybox ────────────────────────────────────────────────────────────
    sky.render(view, projection);

    // ── Mundo ─────────────────────────────────────────────────────────────
    sky.applyToShader(worldShader);
    player.renderCamera(worldShader, screenW, screenH);
    world.render(worldShader);

    // ── Block outline ─────────────────────────────────────────────────────
    blockOutline.render(
        player.targetBlock,
        player.getView(),
        player.getProjection(screenW, screenH)
    );

    // ── HUD (sempre por cima, sem depth) ──────────────────────────────────
    hud.render(player.hotbar, BlockRegistry::getTextureArray(), screenW, screenH);

    // ── Crosshair ─────────────────────────────────────────────────────────
    drawCrosshair();
}

void GameScene::initCrosshair() {
    float verts[] = {
        -0.015f,  0.0f,   0.0f,
         0.015f,  0.0f,   0.0f,
         0.0f,   -0.015f, 0.0f,
         0.0f,    0.015f, 0.0f,
    };
    glGenVertexArrays(1, &crosshairVAO);
    glGenBuffers(1, &crosshairVBO);
    glBindVertexArray(crosshairVAO);
    glBindBuffer(GL_ARRAY_BUFFER, crosshairVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
}

void GameScene::drawCrosshair() {
    uiShader.use();
    float aspect = (float)screenW / (float)screenH;
    uiShader.setFloat("aspectRatio", aspect);
    glBindVertexArray(crosshairVAO);
    glDrawArrays(GL_LINES, 0, 4);
    glBindVertexArray(0);
}