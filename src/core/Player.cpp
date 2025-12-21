#include "../headers/Player.h"

void Player::update(float dt, Input& input) {

    glm::vec3 forward = glm::normalize(glm::vec3(
        cos(glm::radians(camera.yaw)), 
        0.0f,
        sin(glm::radians(camera.yaw))
    ));

    glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0,1,0)));

    if (input.forward) position += forward * speed * dt;
    if (input.back)    position -= forward * speed * dt;
    if (input.left)    position -= right   * speed * dt;
    if (input.right)   position += right   * speed * dt;

    camera.rotate(input.mouseDX, input.mouseDY);

    applyGravity(dt);

    camera.position = position;

    const_cast<Input&>(input).mouseDX = 0.0f;
    const_cast<Input&>(input).mouseDY = 0.0f;
}


void Player::renderCamera(Shader& shader, int w, int h) {
    shader.use();
    shader.setMat4("view", camera.getView());
    shader.setMat4("projection", camera.getProjection(w, h));
}

void Player::applyGravity(float dt) {
    velocity.y += gravity * dt;
    position += velocity * dt;

    // chão simples
    if (position.y < 16.0f) { 

        position.y = 16.0f;
        velocity.y = 0.0f;
    }
}

