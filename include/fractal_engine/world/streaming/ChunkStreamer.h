#pragma once
#include "fractal_engine/world/World.h"
#include <third_party/glm/glm.hpp>
#include <queue>
#include <vector>

namespace fractal_engine::world {

// ─────────────────────────────────────────────────────────────────────────────
// ChunkStreamer — decide QUAIS chunks carregar/descarregar e EM QUE ORDEM
//
// Enfileira chunks em espiral (mais próximos do player primeiro) e processa
// N chunks por frame para evitar spike de CPU.
//
// Use WorldManager para o caso simples (carregamento síncrono).
// Use ChunkStreamer quando quiser controle fino por frame.
// ─────────────────────────────────────────────────────────────────────────────
class ChunkStreamer {
public:
    ChunkStreamer(World& world, Shader& shader, int renderDistance = 4);

    // Detecta mudança de chunk e enfileira carregamentos/descarregamentos
    void update(glm::vec3 playerPos);

    // Processa N chunks da fila por frame (evita spike)
    void processQueue(int chunksPerFrame = 2);

    // Carregamento imediato de todos os chunks (uso no spawn)
    void loadImmediate(glm::vec3 center);

    int    getRenderDistance() const       { return renderDistance; }
    void   setRenderDistance(int d)        { renderDistance = d; }
    size_t queueSize()         const       { return loadQueue.size(); }

private:
    World&  world;
    Shader& shader;
    int     renderDistance;

    glm::ivec3 lastChunk { INT_MAX, 0, INT_MAX };

    struct ChunkRequest {
        glm::ivec3 key;
        float      priority;  // menor = mais urgente (distância ao player)
        bool operator>(const ChunkRequest& o) const { return priority > o.priority; }
    };

    std::priority_queue<
        ChunkRequest,
        std::vector<ChunkRequest>,
        std::greater<ChunkRequest>
    > loadQueue;

    glm::ivec3 worldToChunkKey    (glm::vec3 pos) const;
    void       enqueueChunksAround(glm::ivec3 center);
    void       unloadDistant      (glm::ivec3 center);
};

} // namespace fractal_engine::world