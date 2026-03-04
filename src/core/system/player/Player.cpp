#include "Player.h"
#include <iostream>

void Player::update(float dt, Input& input, World& world) {
    // 1. Rotação da câmera
    camera.rotate(input.mouseDX, input.mouseDY);

    // Direções de movimento no plano XZ
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

    // =============================
    // DETECÇÃO DE CHÃO
    // =============================
    // Pequeno offset para verificar se há bloco logo abaixo
    position.y -= 0.05f;
    if (checkCollision(world))
        onGround = true;
    else
        onGround = false;
    position.y += 0.05f;

    // =============================
    // PULO
    // =============================
    if (input.jump && onGround)
    {
        velocity.y = jumpForce;
        onGround = false;
    }

    // =============================
    // GRAVIDADE
    // =============================
    velocity.y += gravity * dt;

    // =============================
    // MOVIMENTO X
    // =============================
    position.x += moveDir.x * speed * dt;

    if (checkCollision(world))
    {
        position.x -= moveDir.x * speed * dt;
    }

    // =============================
    // MOVIMENTO Z
    // =============================
    position.z += moveDir.z * speed * dt;

    if (checkCollision(world))
    {
        position.z -= moveDir.z * speed * dt;
    }

    // =============================
    // MOVIMENTO Y
    // =============================
    position.y += velocity.y * dt;

    if (checkCollision(world))
    {
        if (velocity.y < 0.0f)
            onGround = true;

        position.y -= velocity.y * dt;
        velocity.y = 0.0f;
    }

    // =============================
    // Atualiza câmera
    // =============================
    camera.position = position + glm::vec3(0, 1.6f, 0);

    // Reset mouse delta
    input.mouseDX = 0.0f;
    input.mouseDY = 0.0f;
}

bool Player::checkCollision(World& world) {
    AABB box = getAABB();
    
    // Checar todos os blocos que a caixa do jogador toca
    for (int x = std::floor(box.min.x); x < std::ceil(box.max.x); x++) {
        for (int y = std::floor(box.min.y); y < std::ceil(box.max.y); y++) {
            for (int z = std::floor(box.min.z); z < std::ceil(box.max.z); z++) {
                if (world.isBlockSolid(x, y, z)) {
                    return true; // Colidiu com algo sólido
                }
            }
        }
    }
    return false;
}


void Player::renderCamera(Shader& shader, int w, int h) {

    shader.use();
    shader.setMat4("view", camera.getView());
    shader.setMat4("projection", camera.getProjection(w, h));
}

void Player::applyGravity(float dt) {

    velocity.y += gravity * dt;
    position += velocity * dt;

    // simple floor
    if (position.y < 4.0f) { 

        position.y = 4.0f;
        velocity.y = 0.0f;
    }
}