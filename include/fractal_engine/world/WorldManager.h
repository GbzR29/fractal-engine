#pragma once
#include "fractal_engine/world/World.h"
#include "fractal_engine/world/terrain/HeightMap.h"
#include "fractal_engine/world/terrain/TerrainGenerator.h"
#include <third_party/glm/glm.hpp>

namespace fractal_engine::world {

// ─────────────────────────────────────────────────────────────────────────────
// WorldManager — gerencia o ciclo de vida dos chunks ao redor do player
//
// Responsabilidades:
//   - Carregar chunks dentro do render distance quando o player se move
//   - Descarregar chunks muito distantes para liberar memória
//   - Calcular posição de spawn segura (acima da superfície)
// ─────────────────────────────────────────────────────────────────────────────
class WorldManager {
public:
    struct Config {
        int   renderDistanceX  = 4;    // chunks em cada direção X
        int   renderDistanceZ  = 4;    // chunks em cada direção Z
        float unloadMultiplier = 1.5f; // descarrega além de renderDistance × este valor
    };

    WorldManager(World& world, Shader& shader, unsigned int seed = 1337);

    // Deve ser chamado todo frame — carrega/descarrega conforme necessário
    void update(glm::vec3 playerPos);

    // Retorna posição de spawn segura próxima à origem (0, ?, 0)
    glm::vec3 getSpawnPosition();

    void   setConfig(const Config& cfg) { config = cfg; }
    Config getConfig() const            { return config; }

    HeightMap& getHeightMap() { return heightMap; }

private:
    World&          world;
    Shader&         shader;
    TerrainGenerator terrainGen;
    HeightMap        heightMap;
    Config           config;

    glm::ivec3 lastPlayerChunk { INT_MAX, 0, INT_MAX };

    glm::ivec3 worldToChunkKey   (glm::vec3 pos) const;
    void       loadChunksAround  (glm::ivec3 center);
    void       unloadDistantChunks(glm::ivec3 center);
};

} // namespace fractal_engine::world