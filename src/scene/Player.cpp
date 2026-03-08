#include "fractal_engine/scene/Player.h"
#include <iostream>
#include <cmath>

namespace fractal_engine::scene {

void Player::update(float dt, Input& input, World& world) {
    // ── Câmera (rotação) ──────────────────────────────────────────────────
    camera.rotate(input.mouseDX, input.mouseDY);
    input.mouseDX = 0.0f;
    input.mouseDY = 0.0f;

    // ── Vetores de direção (horizontal apenas) ────────────────────────────
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

    // ── Hotbar ────────────────────────────────────────────────────────────
    hotbar.update(input);

    // ── Detecção de chão ──────────────────────────────────────────────────
    // Testa um pequeno deslocamento para baixo — robusto mesmo com dt variável
    {
        position.y -= 0.001f;
        onGround = checkCollision(world);
        position.y += 0.001f;
    }

    // ── Pulo ──────────────────────────────────────────────────────────────
    if (input.jump && onGround) {
        velocity.y = jumpForce;
        onGround   = false;
    }

    // ── Gravidade ─────────────────────────────────────────────────────────
    // Usa gravidade forte (escala de voxels ≈ 1m/bloco)
    // Terminal velocity de ~-50 evita atravessar blocos em frames lentos
    if (!onGround) {
        velocity.y += gravity * dt;
        if (velocity.y < -50.0f) velocity.y = -50.0f;
    }

    // ── Movimento X ───────────────────────────────────────────────────────
    {
        float dx = moveDir.x * speed * dt;
        position.x += dx;
        if (checkCollision(world)) {
            position.x -= dx;
            // Tenta deslizar: move apenas em X puro sem diagonal
            position.x += moveDir.x * speed * dt * 0.5f;
            if (checkCollision(world))
                position.x -= moveDir.x * speed * dt * 0.5f;
        }
    }

    // ── Movimento Z ───────────────────────────────────────────────────────
    {
        float dz = moveDir.z * speed * dt;
        position.z += dz;
        if (checkCollision(world)) {
            position.z -= dz;
            position.z += moveDir.z * speed * dt * 0.5f;
            if (checkCollision(world))
                position.z -= moveDir.z * speed * dt * 0.5f;
        }
    }

    // ── Movimento Y (gravidade + pulo) ────────────────────────────────────
    {
        float dy = velocity.y * dt;
        position.y += dy;
        if (checkCollision(world)) {
            // Resolve colisão vertical bloco a bloco para não enterrar
            // em alta velocidade
            position.y -= dy;
            float step = (dy > 0.0f ? 1.0f : -1.0f) * 0.05f;
            int   maxSteps = (int)(std::abs(dy) / 0.05f) + 1;
            for (int i = 0; i < maxSteps; i++) {
                position.y += step;
                if (checkCollision(world)) {
                    position.y -= step;
                    break;
                }
            }
            if (velocity.y < 0.0f) onGround = true;
            velocity.y = 0.0f;
        }
    }

    // ── Câmera — segue o player ───────────────────────────────────────────
    camera.position = position + glm::vec3(0.0f, eyeHeight, 0.0f);

    // ── Raycast e interação ───────────────────────────────────────────────
    targetBlock = Raycast::raycast(world, camera.position,
                                   camera.getFront(), blockInteractionRange);

    if (input.leftClickPressed() && targetBlock.has_value())
        world.breakBlock(targetBlock->blockPos);

    if (input.rightClickPressed() && targetBlock.has_value()) {
        // Só coloca o bloco se não colidir com o player
        glm::ivec3 placePos = targetBlock->adjacentPos;
        if (wouldCollideAt(placePos, world)) {
            // Bloco colidiria com o player — ignora
        } else {
            world.placeBlock(placePos, hotbar.getSelectedBlock());
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// checkCollision — AABB vs blocos sólidos do mundo
// ─────────────────────────────────────────────────────────────────────────────
bool Player::checkCollision(World& world) {
    AABB box = getAABB();
    static constexpr float EPS = 0.001f;

    int minX = static_cast<int>(std::floor(box.min.x + EPS));
    int maxX = static_cast<int>(std::floor(box.max.x - EPS));
    int minY = static_cast<int>(std::floor(box.min.y + EPS));
    int maxY = static_cast<int>(std::floor(box.max.y - EPS));
    int minZ = static_cast<int>(std::floor(box.min.z + EPS));
    int maxZ = static_cast<int>(std::floor(box.max.z - EPS));

    for (int x = minX; x <= maxX; x++)
    for (int y = minY; y <= maxY; y++)
    for (int z = minZ; z <= maxZ; z++) {
        if (world.isBlockSolid((float)x, (float)y, (float)z))
            return true;
    }
    return false;
}

// ─────────────────────────────────────────────────────────────────────────────
// wouldCollideAt — checa se colocar um bloco em `pos` colidiria com o player
// ─────────────────────────────────────────────────────────────────────────────
bool Player::wouldCollideAt(glm::ivec3 blockPos, World& world) const {
    AABB box = getAABB();

    // AABB do bloco a ser colocado (1×1×1 centrado em blockPos)
    glm::vec3 bMin = glm::vec3(blockPos);
    glm::vec3 bMax = bMin + glm::vec3(1.0f);

    // Teste de intersecção AABB vs AABB
    return (box.min.x < bMax.x && box.max.x > bMin.x)
        && (box.min.y < bMax.y && box.max.y > bMin.y)
        && (box.min.z < bMax.z && box.max.z > bMin.z);
}

// ─────────────────────────────────────────────────────────────────────────────
// renderCamera
// ─────────────────────────────────────────────────────────────────────────────
void Player::renderCamera(Shader& shader, int w, int h) {
    shader.use();
    shader.setMat4("view",       camera.getView());
    shader.setMat4("projection", camera.getProjection(w, h));
}

// ─────────────────────────────────────────────────────────────────────────────
// applyGravity — mantido por compatibilidade mas não usado no update()
// ─────────────────────────────────────────────────────────────────────────────
void Player::applyGravity(float dt) {
    velocity.y += gravity * dt;
    position   += velocity * dt;
    if (position.y < 4.0f) { position.y = 4.0f; velocity.y = 0.0f; }
}

// ─────────────────────────────────────────────────────────────────────────────
// initializeAtTerrainHeight
// ─────────────────────────────────────────────────────────────────────────────
void Player::initializeAtTerrainHeight(const World& world,
                                        float spawnX, float spawnZ) {
    for (int y = Chunk::SIZE_Y - 2; y >= 1; y--) {
        if (world.isBlockSolid(spawnX, (float)y, spawnZ)) {
            position = glm::vec3(spawnX, (float)(y + 1), spawnZ);
            velocity = glm::vec3(0.0f);
            onGround = false;
            std::cout << "[Player] Spawn Y=" << y
                      << " → position.y=" << position.y << "\n";
            return;
        }
    }
    position = glm::vec3(spawnX, (float)(Chunk::SIZE_Y - 2), spawnZ);
    velocity = glm::vec3(0.0f);
    onGround = false;
    std::cout << "[Player] AVISO: fallback spawn Y=" << position.y << "\n";
}

} // namespace fractal_engine::scene