#include "fractal_engine/scene/Player.h"
#include <iostream>

namespace fractal_engine::scene {

void Player::update(float dt, Input& input, World& world) {
    camera.rotate(input.mouseDX, input.mouseDY);

    glm::vec3 forward = glm::normalize(
        glm::vec3(cos(glm::radians(camera.yaw)), 0.0f, sin(glm::radians(camera.yaw)))
    );
    glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0, 1, 0)));

    glm::vec3 moveDir(0.0f);
    if (input.forward) moveDir += forward;
    if (input.back)    moveDir -= forward;
    if (input.left)    moveDir -= right;
    if (input.right)   moveDir += right;
    if (glm::length(moveDir) > 0.0f)
        moveDir = glm::normalize(moveDir);

    // ── Detecção de chão ──────────────────────────────────────────────────
    position.y -= 0.05f;
    onGround = checkCollision(world);
    position.y += 0.05f;

    // ── Pulo ──────────────────────────────────────────────────────────────
    if (input.jump && onGround) {
        velocity.y = jumpForce;
        onGround   = false;
    }

    // ── Gravidade ─────────────────────────────────────────────────────────
    velocity.y += gravity * dt;

    // ── Movimento X ───────────────────────────────────────────────────────
    position.x += moveDir.x * speed * dt;
    if (checkCollision(world))
        position.x -= moveDir.x * speed * dt;

    // ── Movimento Z ───────────────────────────────────────────────────────
    position.z += moveDir.z * speed * dt;
    if (checkCollision(world))
        position.z -= moveDir.z * speed * dt;

    // ── Movimento Y ───────────────────────────────────────────────────────
    position.y += velocity.y * dt;
    if (checkCollision(world)) {
        if (velocity.y < 0.0f) onGround = true;
        position.y -= velocity.y * dt;
        velocity.y = 0.0f;
    }

    // ── Câmera ────────────────────────────────────────────────────────────
    camera.position = position + glm::vec3(0, 1.6f, 0);

    // ── Raycast e interação ───────────────────────────────────────────────
    targetBlock = Raycast::raycast(world, camera.position, camera.getFront(), blockInteractionRange);

    if (input.leftClickPressed() && targetBlock.has_value())
        world.breakBlock(targetBlock->blockPos);

    if (input.rightClickPressed() && targetBlock.has_value())
        world.placeBlock(targetBlock->adjacentPos, selectedBlockType);

    input.mouseDX = 0.0f;
    input.mouseDY = 0.0f;

    if (input.key1Pressed) selectedBlockType = BLOCK_STONE;
    if (input.key2Pressed) selectedBlockType = BLOCK_GRASS;
    if (input.key3Pressed) selectedBlockType = BLOCK_DIRT;
}

bool Player::checkCollision(World& world) {
    AABB box = getAABB();

    int minX = static_cast<int>(std::floor(box.min.x));
    int maxX = static_cast<int>(std::ceil (box.max.x));
    int minY = static_cast<int>(std::floor(box.min.y));
    int maxY = static_cast<int>(std::ceil (box.max.y));
    int minZ = static_cast<int>(std::floor(box.min.z));
    int maxZ = static_cast<int>(std::ceil (box.max.z));

    for (int x = minX; x <= maxX; x++)
    for (int y = minY; y <= maxY; y++)
    for (int z = minZ; z <= maxZ; z++) {
        if (world.isBlockSolid((float)x, (float)y, (float)z)) {
            if (!(box.max.x <= x      || box.min.x >= x + 1.0f ||
                  box.max.y <= y      || box.min.y >= y + 1.0f ||
                  box.max.z <= z      || box.min.z >= z + 1.0f))
                return true;
        }
    }
    return false;
}

void Player::renderCamera(Shader& shader, int w, int h) {
    shader.use();
    shader.setMat4("view",       camera.getView());
    shader.setMat4("projection", camera.getProjection(w, h));
}

void Player::applyGravity(float dt) {
    velocity.y += gravity * dt;
    position   += velocity * dt;
    if (position.y < 4.0f) { position.y = 4.0f; velocity.y = 0.0f; }
}

// ─────────────────────────────────────────────────────────────────────────────
// initializeAtTerrainHeight — abordagem "drop from sky"
//
// Em vez de tentar calcular matematicamente onde a superfície está
// (o que pode divergir dependendo de como o fractalNoise normaliza valores),
// simplesmente varremos de cima para baixo nos blocos reais do mundo.
//
// Se o scan falhar (chunk não carregado ainda), colocamos o player no
// topo absoluto (Chunk::SIZE_Y - 2) e a gravidade + colisão cuidam do resto.
// ─────────────────────────────────────────────────────────────────────────────
void Player::initializeAtTerrainHeight(const World& world, float spawnX, float spawnZ) {
    // Varrer de cima para baixo procurando o primeiro bloco sólido
    // Começa em SIZE_Y-2 para deixar espaço para o player (altura 2)
    int topY = Chunk::SIZE_Y - 2;

    for (int y = topY; y >= 1; y--) {
        if (world.isBlockSolid(spawnX, (float)y, spawnZ)) {
            // Encontrou terreno: posiciona os PÉS do player em y+1
            // (y é o bloco sólido, y+1 é o ar acima dele)
            position = glm::vec3(spawnX, (float)(y + 1), spawnZ);
            velocity = glm::vec3(0.0f);
            onGround = false;

            std::cout << "[Player] Spawn no bloco solido Y=" << y
                      << " → player em Y=" << position.y << "\n";
            return;
        }
    }

    // Fallback: nenhum bloco encontrado (chunk vazio ou não carregado)
    // Spawn no topo — a gravidade vai pousar o player quando o chunk carregar
    position = glm::vec3(spawnX, (float)(Chunk::SIZE_Y - 2), spawnZ);
    velocity = glm::vec3(0.0f);
    onGround = false;

    std::cout << "[Player] AVISO: nenhum bloco encontrado, spawn no topo Y="
              << position.y << "\n";
}

} // namespace fractal_engine::scene