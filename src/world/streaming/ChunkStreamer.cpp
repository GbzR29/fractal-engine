#include "fractal_engine/world/streaming/ChunkStreamer.h"
#include "fractal_engine/world/terrain/Chunk.h"
#include <cmath>
#include <iostream>

namespace fractal_engine::world {

ChunkStreamer::ChunkStreamer(World& world, Shader& shader, int renderDistance)
    : world(world)
    , shader(shader)
    , renderDistance(renderDistance)
{}

// ─────────────────────────────────────────────
glm::ivec3 ChunkStreamer::worldToChunkKey(glm::vec3 pos) const {
    auto floorDiv = [](int a, int b) -> int {
        return a / b - (a % b != 0 && (a ^ b) < 0);
    };
    int cx = floorDiv((int)std::floor(pos.x), Chunk::SIZE_X) * Chunk::SIZE_X;
    int cz = floorDiv((int)std::floor(pos.z), Chunk::SIZE_Z) * Chunk::SIZE_Z;
    return glm::ivec3(cx, 0, cz);
}

// ─────────────────────────────────────────────
void ChunkStreamer::update(glm::vec3 playerPos) {
    glm::ivec3 currentChunk = worldToChunkKey(playerPos);
    if (currentChunk == lastChunk) return;

    lastChunk = currentChunk;
    enqueueChunksAround(currentChunk);
    unloadDistant(currentChunk);
}

// ─────────────────────────────────────────────
// Enfileira chunks em espiral (mais próximos primeiro)
// ─────────────────────────────────────────────
void ChunkStreamer::enqueueChunksAround(glm::ivec3 center) {
    const int rd = renderDistance;

    for (int cx = -rd; cx <= rd; cx++) {
        for (int cz = -rd; cz <= rd; cz++) {
            glm::ivec3 key(
                center.x + cx * Chunk::SIZE_X,
                0,
                center.z + cz * Chunk::SIZE_Z
            );

            // Não enfileira se já existe
            if (world.getChunks().count(key)) continue;

            float dist = std::sqrt((float)(cx*cx + cz*cz));
            loadQueue.push({ key, dist });
        }
    }
}

// ─────────────────────────────────────────────
// Processa N chunks por frame da fila
// ─────────────────────────────────────────────
void ChunkStreamer::processQueue(int chunksPerFrame) {
    for (int i = 0; i < chunksPerFrame && !loadQueue.empty(); i++) {
        auto req = loadQueue.top();
        loadQueue.pop();

        // Double-check: pode ter sido criado por outro caminho
        if (!world.getChunks().count(req.key)) {
            world.addChunk(req.key, shader);
        }
    }
}

// ─────────────────────────────────────────────
// Carregamento imediato — usado no spawn
// ─────────────────────────────────────────────
void ChunkStreamer::loadImmediate(glm::vec3 center) {
    glm::ivec3 centerChunk = worldToChunkKey(center);
    enqueueChunksAround(centerChunk);

    // Processa tudo de uma vez
    while (!loadQueue.empty()) {
        auto req = loadQueue.top();
        loadQueue.pop();
        if (!world.getChunks().count(req.key)) {
            world.addChunk(req.key, shader);
        }
    }
}

// ─────────────────────────────────────────────
// Descarrega chunks longe demais
// ─────────────────────────────────────────────
void ChunkStreamer::unloadDistant(glm::ivec3 center) {
    const float maxDist = (float)(renderDistance + 2) * (float)Chunk::SIZE_X;

    std::vector<glm::ivec3> toRemove;
    for (auto& [key, chunk] : world.getChunks()) {
        float dx = (float)std::abs(key.x - center.x);
        float dz = (float)std::abs(key.z - center.z);
        if (std::sqrt(dx*dx + dz*dz) > maxDist) {
            toRemove.push_back(key);
        }
    }

    for (auto& key : toRemove) {
        world.removeChunk(key);
    }
}

} // namespace fractal_engine::world