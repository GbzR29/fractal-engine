#pragma once

#include <third_party/glm/glm.hpp>
#include <third_party/glm/gtc/matrix_transform.hpp>
#include <optional>
#include "fractal_engine/graphics/Camera.h"
#include "fractal_engine/input/Input.h"
#include "fractal_engine/world/World.h"
#include "fractal_engine/world/Raycast.h"

using namespace fractal_engine::graphics;
using namespace fractal_engine::input;
using namespace fractal_engine::world;

namespace fractal_engine::scene {

struct AABB {
    glm::vec3 min, max;
};

class Player {

    float width = 0.8f;   // Aumentado de 0.6f para corpo maior
    float height = 2.0f;  // Aumentado de 1.8f para corpo maior
    
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

    /**
     * @brief Inicializa a posição do player acima do terreno
     * 
     * Encontra a altura do terreno na posição X,Z especificada e coloca o player lá
     * @param world Referência ao mundo para calcular altura do terreno
     * @param spawnX Coordenada X de spawn
     * @param spawnZ Coordenada Z de spawn
     */
    void initializeAtTerrainHeight(const World& world, float spawnX = 0.0f, float spawnZ = 0.0f);

    bool onGround = false;
    float jumpForce = 6.0f;

    // ─────────────────────────────────────────────
    // SISTEMA DE QUEBRA/CONSTRUÇÃO
    // ─────────────────────────────────────────────
    
    // Bloco atualmente sendo olhado (para render de seleção)
    std::optional<RaycastHit> targetBlock;
    
    // Alcance máximo de interação com blocos (em blocos)
    float blockInteractionRange = 5.0f;
    
    // Tipo de bloco selecionado para construir
    BlockType selectedBlockType = BLOCK_STONE;  // Padrão: pedra

private:
    Camera camera;

    glm::vec3 position{0.0f, 64.0f, 0.0f};  // Altura muito acima do terreno (BASE_HEIGHT=32, variação ±48)
    glm::vec3 velocity{0.0f};

    float speed   = 5.0f;
    float gravity = -9.8f;
};

} // namespace fractal_engine::scene
