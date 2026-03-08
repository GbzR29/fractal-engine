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
    // ── Dimensões do player ───────────────────────────────────────────────
    static constexpr float width     = 0.6f;
    static constexpr float height    = 1.8f;
    static constexpr float eyeHeight = 1.6f;

    AABB getAABB() const {
        return {
            position - glm::vec3(width * 0.5f, 0.0f,   width * 0.5f),
            position + glm::vec3(width * 0.5f, height, width * 0.5f)
        };
    }

public:
    Player() = default;

    void init() { hotbar.init(); }

    void update(float dt, Input& input, World& world);
    void applyGravity(float dt);
    void renderCamera(Shader& shader, int w, int h);
    bool checkCollision(World& world);

    // Retorna true se colocar um bloco em blockPos colidiria com o player
    bool wouldCollideAt(glm::ivec3 blockPos, World& world) const;

    void initializeAtTerrainHeight(const World& world,
                                   float spawnX = 0.0f,
                                   float spawnZ = 0.0f);

    glm::mat4 getView()                   const { return camera.getView(); }
    glm::mat4 getProjection(int w, int h) const { return camera.getProjection(w, h); }

    // ── Estado público ────────────────────────────────────────────────────
    bool  onGround  = false;

    // ── Interação ─────────────────────────────────────────────────────────
    std::optional<RaycastHit> targetBlock;
    float blockInteractionRange = 5.0f;

    // ── Hotbar ────────────────────────────────────────────────────────────
    Hotbar hotbar;

private:
    Camera    camera;

    glm::vec3 position { 0.0f, 64.0f, 0.0f };
    glm::vec3 velocity { 0.0f };

    // ── Constantes de física (escala: 1 bloco = 1 metro) ─────────────────
    float speed     =  8.0f;   // blocos/s horizontal
    float gravity   = -28.0f;  // m/s² (Minecraft usa ~32)
    float jumpForce =  9.0f;   // velocidade inicial do pulo (m/s)
};

} // namespace fractal_engine::scene