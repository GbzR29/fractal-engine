#pragma once

#include <third_party/glm/glm.hpp>
#include <third_party/glm/gtc/matrix_transform.hpp>
#include "fractal_engine/graphics/Camera.h"
#include "fractal_engine/input/Input.h"
#include "fractal_engine/world/World.h"

using namespace fractal_engine::graphics;
using namespace fractal_engine::input;
using namespace fractal_engine::world;

namespace fractal_engine::scene {

struct AABB {
    glm::vec3 min, max;
};

class Player {

    float width = 0.6f;
    float height = 1.8f;
    
    AABB getAABB() {
        return {
            position - glm::vec3(width/2, 0, width/2), // min (pés)
            position + glm::vec3(width/2, height, width/2) // max (cabeça)
        };
    }

public:
    Player() = default;

    void update(float dt, Input& input, World& world);
    void applyGravity(float dt);

    void renderCamera(Shader& shader, int w, int h);

    bool checkCollision(World& world);

    bool onGround = false;
    float jumpForce = 6.0f;

private:
    Camera camera;

    glm::vec3 position{0.0f, 30.0f, 0.0f};
    glm::vec3 velocity{0.0f};

    float speed   = 5.0f;
    float gravity = -9.8f;
};

} // namespace fractal_engine::scene
