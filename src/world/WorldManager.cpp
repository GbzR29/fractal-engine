#include "fractal_engine/world/WorldManager.h"
#include "fractal_engine/world/terrain/Chunk.h"
#include <cmath>
#include <iostream>

namespace fractal_engine::world {

WorldManager::WorldManager(World& world, Shader& shader, unsigned int seed)
    : world(world)
    , shader(shader)
    , heightMap(TerrainGenerator(seed))
    , terrainGen(seed)
{}

// ─────────────────────────────────────────────
// Converte posição de mundo para chave de chunk
// ─────────────────────────────────────────────
glm::ivec3 WorldManager::worldToChunkKey(glm::vec3 pos) const {
    auto floorDiv = [](int a, int b) -> int {
        return a / b - (a % b != 0 && (a ^ b) < 0);
    };
    int cx = floorDiv((int)std::floor(pos.x), Chunk::SIZE_X) * Chunk::SIZE_X;
    int cz = floorDiv((int)std::floor(pos.z), Chunk::SIZE_Z) * Chunk::SIZE_Z;
    return glm::ivec3(cx, 0, cz);
}

// ─────────────────────────────────────────────
// Update — chamado todo frame com posição do player
// ─────────────────────────────────────────────
void WorldManager::update(glm::vec3 playerPos) {
    glm::ivec3 currentChunk = worldToChunkKey(playerPos);

    // Só atualiza se o player entrou em um novo chunk
    if (currentChunk == lastPlayerChunk) return;
    lastPlayerChunk = currentChunk;

    loadChunksAround(currentChunk);
    unloadDistantChunks(currentChunk);
}

// ─────────────────────────────────────────────
// Carrega chunks faltando ao redor do player
// ─────────────────────────────────────────────
void WorldManager::loadChunksAround(glm::ivec3 centerChunk) {
    const int rdX = config.renderDistanceX;
    const int rdZ = config.renderDistanceZ;

    for (int cx = -rdX; cx <= rdX; cx++) {
        for (int cz = -rdZ; cz <= rdZ; cz++) {
            glm::ivec3 key(
                centerChunk.x + cx * Chunk::SIZE_X,
                0,
                centerChunk.z + cz * Chunk::SIZE_Z
            );

            // addChunk ignora se o chunk já existe
            world.addChunk(key, shader);
        }
    }
}

// ─────────────────────────────────────────────
// Remove chunks longe demais para liberar memória
// ─────────────────────────────────────────────
void WorldManager::unloadDistantChunks(glm::ivec3 centerChunk) {
    const float maxDistX = (float)(config.renderDistanceX * Chunk::SIZE_X) * config.unloadMultiplier;
    const float maxDistZ = (float)(config.renderDistanceZ * Chunk::SIZE_Z) * config.unloadMultiplier;

    // Coleta chunks a remover (não remove durante iteração)
    std::vector<glm::ivec3> toRemove;

    for (auto& [key, chunk] : world.getChunks()) {
        float dx = std::abs((float)(key.x - centerChunk.x));
        float dz = std::abs((float)(key.z - centerChunk.z));

        if (dx > maxDistX || dz > maxDistZ) {
            toRemove.push_back(key);
        }
    }

    for (auto& key : toRemove) {
        world.removeChunk(key);
        std::cout << "[WorldManager] Unloaded chunk " 
                  << key.x << "," << key.z << "\n";
    }
}

// ─────────────────────────────────────────────
// Calcula spawn seguro na origem
// ─────────────────────────────────────────────
glm::vec3 WorldManager::getSpawnPosition() {
    // Garante que o chunk da origem existe antes de consultar altura
    world.addChunk(glm::ivec3(0, 0, 0), shader);

    float spawnY = world.getTerrainHeightAt(0.5f, 0.5f, (float)(Chunk::SIZE_Y - 1));

    // +1.8 = altura dos olhos do player (pé em spawnY, cabeça em spawnY+1.8)
    return glm::vec3(0.5f, spawnY + 0.1f, 0.5f);
}

} // namespace fractal_engine::world