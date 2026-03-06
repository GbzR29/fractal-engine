#pragma once
#include <optional>
#include <glm/glm.hpp>

namespace fractal_engine::world {

class World;

// ─────────────────────────────────────────────────────────────
// RaycastHit — resultado de um raycast contra o mundo de voxels
// ─────────────────────────────────────────────────────────────
struct RaycastHit {
    glm::ivec3 blockPos;     // Bloco sólido atingido
    glm::ivec3 adjacentPos;  // Bloco de ar anterior ao hit (onde colocar bloco)
    glm::ivec3 faceNormal;   // Normal da face atingida: ex. (0,1,0) = face top
                              //   Use: adjacentPos = blockPos + faceNormal
    float      distance;     // Distância da origem até a face do bloco
};

// ─────────────────────────────────────────────────────────────
class Raycast {
public:
    // origin    : posição dos olhos do player (ex: playerPos + {0, 1.6f, 0})
    // direction : vetor de direção da câmera (não precisa ser normalizado)
    // maxDistance: alcance máximo em unidades de bloco (ex: 6.0f)
    static std::optional<RaycastHit> raycast(
        const World& world,
        glm::vec3    origin,
        glm::vec3    direction,
        float        maxDistance = 6.0f
    );
};

} // namespace fractal_engine::world