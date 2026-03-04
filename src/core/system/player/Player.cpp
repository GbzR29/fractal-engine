#include "Player.h"

void Player::update(float dt, Input& input, World& world) {
    // 1. Rotação (independente de colisão)
    camera.rotate(input.mouseDX, input.mouseDY);

    // 2. Aplicar Gravidade
    velocity.y += gravity * dt;

    // 2. Cálculo de movimento desejado
    glm::vec3 forward = glm::normalize(glm::vec3(cos(glm::radians(camera.yaw)), 0.0f, sin(glm::radians(camera.yaw))));
    glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0,1,0)));
    
    glm::vec3 moveDir(0.0f);
    if (input.forward) moveDir += forward;
    if (input.back)    moveDir -= forward;
    if (input.left)    moveDir -= right;
    if (input.right)   moveDir += right;
    
    if (glm::length(moveDir) > 0) moveDir = glm::normalize(moveDir);

    // 3. Mover Eixo X
    position.x += moveDir.x * speed * dt;
    if (checkCollision(world)) {
        position.x -= moveDir.x * speed * dt; // Undo
    }

    // 4. Mover Eixo Z
    position.z += moveDir.z * speed * dt;
    if (checkCollision(world)) {
        position.z -= moveDir.z * speed * dt; // Undo
    }

    // 5. Gravidade e Eixo Y
    velocity.y += gravity * dt;

    // 3. Pulo (Apenas se estiver no chão)
    if (input.jump && onGround) {
        // A velocidade vertical recebe o impulso instantâneo
        velocity.y = jumpForce; 
        onGround = false; // Ao pular, ele não está mais no chão
    }

    position.y += velocity.y * dt;

    if (checkCollision(world)) {
        // Se houve colisão e ele estava caindo (vel < 0), ele bateu no chão
        if (velocity.y < 0) {
            onGround = true;
        }
        
        // Resolve a colisão (volta para a posição anterior e anula a velocidade)
        position.y -= velocity.y * dt;
        velocity.y = 0;
    }

    bool wasOnGround = onGround;
    onGround = false; 

    camera.position = position + glm::vec3(0, 1.6f, 0); // Olhos na altura da cabeça
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