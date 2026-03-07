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
    // Olhos a 1.6f acima dos pés — padrão Minecraft (player tem 1.8 blocos visuais)
    camera.position = position + glm::vec3(0, eyeHeight, 0);

    // ── Raycast e interação ───────────────────────────────────────────────
    targetBlock = Raycast::raycast(world, camera.position, camera.getFront(), blockInteractionRange);

    if (input.leftClickPressed()  && targetBlock.has_value())
        world.breakBlock(targetBlock->blockPos);

    if (input.rightClickPressed() && targetBlock.has_value())
        world.placeBlock(targetBlock->adjacentPos, selectedBlockType);

    input.mouseDX = 0.0f;
    input.mouseDY = 0.0f;

    if (input.key1Pressed) selectedBlockType = BLOCK_STONE;
    if (input.key2Pressed) selectedBlockType = BLOCK_GRASS;
    if (input.key3Pressed) selectedBlockType = BLOCK_DIRT;
}

// ─────────────────────────────────────────────────────────────────────────────
// checkCollision
//
// FIX: O código anterior usava std::ceil() no max da AABB, o que incluía
// o bloco ACIMA da cabeça do player quando a posição era valor exato
// (ex: maxY = 22.0 → ceil = 22 → checa bloco 22, além do topo real).
//
// Correto: usar std::floor() com epsilon (0.001f) subtraído do max.
// Isso garante que a AABB cobre exatamente os blocos que ela ocupa,
// sem incluir blocos adjacentes nas bordas.
//
// Exemplo com height=2.0f e position.y=20.0:
//   box.max.y = 22.0 → floor(22.0 - 0.001) = floor(21.999) = 21 ✓
//   O player ocupa blocos y=20 e y=21 — correto para 2 blocos de altura.
// ─────────────────────────────────────────────────────────────────────────────
bool Player::checkCollision(World& world) {
    AABB box = getAABB();

    static constexpr float EPS = 0.001f;

    int minX = static_cast<int>(std::floor(box.min.x));
    int maxX = static_cast<int>(std::floor(box.max.x - EPS));
    int minY = static_cast<int>(std::floor(box.min.y));
    int maxY = static_cast<int>(std::floor(box.max.y - EPS));
    int minZ = static_cast<int>(std::floor(box.min.z));
    int maxZ = static_cast<int>(std::floor(box.max.z - EPS));

    for (int x = minX; x <= maxX; x++)
    for (int y = minY; y <= maxY; y++)
    for (int z = minZ; z <= maxZ; z++) {
        if (world.isBlockSolid((float)x, (float)y, (float)z))
            return true;
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

void Player::initializeAtTerrainHeight(const World& world, float spawnX, float spawnZ) {
    int topY = Chunk::SIZE_Y - 2;

    for (int y = topY; y >= 1; y--) {
        if (world.isBlockSolid(spawnX, (float)y, spawnZ)) {
            position = glm::vec3(spawnX, (float)(y + 1), spawnZ);
            velocity = glm::vec3(0.0f);
            onGround = false;

            std::cout << "[Player] Spawn no bloco solido Y=" << y
                      << " → player em Y=" << position.y << "\n";
            return;
        }
    }

    position = glm::vec3(spawnX, (float)(Chunk::SIZE_Y - 2), spawnZ);
    velocity = glm::vec3(0.0f);
    onGround = false;

    std::cout << "[Player] AVISO: nenhum bloco encontrado, spawn no topo Y="
              << position.y << "\n";
}

} // namespace fractal_engine::scene