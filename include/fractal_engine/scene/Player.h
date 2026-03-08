#pragma once
#include <third_party/glm/glm.hpp>
#include <third_party/glm/gtc/matrix_transform.hpp>
#include <optional>
#include "fractal_engine/graphics/Camera.h"
#include "fractal_engine/input/Input.h"
#include "fractal_engine/world/World.h"
#include "fractal_engine/world/Raycast.h"
#include "fractal_engine/scene/Hotbar.h"

using namespace fractal_engine::graphics;
using namespace fractal_engine::input;
using namespace fractal_engine::world;

namespace fractal_engine::scene {

struct AABB {
    glm::vec3 min, max;
};

class Player {
    static constexpr float width     = 0.6f;
    static constexpr float height    = 2.0f;
    static constexpr float eyeHeight = 1.6f;

    AABB getAABB() const {
        return {
            position - glm::vec3(width * 0.5f, 0.0f,   width * 0.5f),
            position + glm::vec3(width * 0.5f, height, width * 0.5f)
        };
    }

public:
    Player() = default;

    // Deve ser chamado após BlockRegistry::init()
    void init() { hotbar.init(); }

    void update(float dt, Input& input, World& world);
    void applyGravity(float dt);
    void renderCamera(Shader& shader, int w, int h);
    bool checkCollision(World& world);
    void initializeAtTerrainHeight(const World& world,
                                   float spawnX = 0.0f,
                                   float spawnZ = 0.0f);

    glm::mat4 getView()             const { return camera.getView(); }
    glm::mat4 getProjection(int w, int h) const { return camera.getProjection(w, h); }

    // ── Estado ────────────────────────────────────────────────────────────
    bool  onGround  = false;
    float jumpForce = 6.0f;

    // ── Interação com blocos ──────────────────────────────────────────────
    std::optional<RaycastHit> targetBlock;
    float blockInteractionRange = 5.0f;

    // ── Hotbar (sem BlockType hardcoded) ──────────────────────────────────
    Hotbar hotbar;

private:
    Camera    camera;
    glm::vec3 position { 0.0f, 64.0f, 0.0f };
    glm::vec3 velocity { 0.0f };
    float     speed    =  5.0f;
    float     gravity  = -9.8f;
};

} // namespace fractal_engine::scene