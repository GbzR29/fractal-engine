#pragma once
#include <optional>
#include <third_party/glm/glm.hpp>

namespace fractal_engine::world {

class World;

// ─────────────────────────────────────────────────────────────────────────────
// RaycastHit — resultado de um raycast contra o mundo de voxels
// ─────────────────────────────────────────────────────────────────────────────
struct RaycastHit {
    glm::ivec3 blockPos;     // bloco SÓLIDO atingido
    glm::ivec3 adjacentPos;  // bloco de AR anterior (onde colocar um novo bloco)
    glm::ivec3 faceNormal;   // normal da face: (0,1,0)=topo (0,-1,0)=fundo etc.
                              // adjacentPos == blockPos + faceNormal
    float      distance;     // distância da origem até a face do bloco
};

// ─────────────────────────────────────────────────────────────────────────────
// Raycast — DDA (Digital Differential Analysis) contra o mundo de voxels
// ─────────────────────────────────────────────────────────────────────────────
class Raycast {
public:
    // origin      : posição dos olhos do player (ex: playerPos + {0, 1.6f, 0})
    // direction   : direção da câmera (não precisa estar normalizado)
    // maxDistance : alcance em blocos (padrão: 6.0f)
    static std::optional<RaycastHit> raycast(
        const World& world,
        glm::vec3    origin,
        glm::vec3    direction,
        float        maxDistance = 6.0f
    );
};

} // namespace fractal_engine::world