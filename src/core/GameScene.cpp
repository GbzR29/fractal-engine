#include "GameScene.h"
#include <iostream>

GameScene::GameScene(Shader& worldShader, Shader& uiShader, int width, int height)
    : worldShader(worldShader), uiShader(uiShader), screenW(width), screenH(height)
{}

void GameScene::init() {
    // 1. Gera o mundo completo (blocos + mesh de todos os chunks)
    world.generateWorld(8, 8, worldShader);

    // 2. Spawn do player DEPOIS do mundo pronto — scan de blocos reais
    player.initializeAtTerrainHeight(world, 0.0f, 0.0f);

    // 3. BlockOutline para visualização de bloco selecionado
    blockOutline.init();

    // 4. UI
    initCrosshair();
}

void GameScene::update(float dt, Input& input, Window& window) {
    player.update(dt, input, world);
}

void GameScene::render(Input& input, Window& window) {
    glClearColor(0.38f, 0.76f, 0.9f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    player.renderCamera(worldShader, screenW, screenH);
    world.render(worldShader);
    
    // ── Renderiza o outline do bloco sendo olhado ──────────────────────────
    blockOutline.render(
        player.targetBlock,
        player.getView(),
        player.getProjection(screenW, screenH)
    );
    
    drawCrosshair();
}

void GameScene::initCrosshair() {
    float verts[] = {
        -0.015f,  0.0f, 0.0f,
         0.015f,  0.0f, 0.0f,
         0.0f, -0.015f, 0.0f,
         0.0f,  0.015f, 0.0f,
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