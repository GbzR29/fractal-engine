#pragma once
#include "fractal_engine/world/BlockType.h"
#include "fractal_engine/world/terrain/TerrainGenerator.h"
#include <third_party/glm/glm.hpp>
#include <thread>
#include <mutex>
#include <queue>
#include <vector>
#include <atomic>
#include <condition_variable>

namespace fractal_engine::world {

// ─────────────────────────────────────────────────────────────────────────────
// AsyncLoader — gera blocos de chunk em threads de background
//
// IMPORTANTE: só a geração de BLOCOS (pura CPU) roda em background.
// O upload para GPU (VAO/VBO) DEVE acontecer na thread principal OpenGL.
//
// Fluxo de uso:
//   1. ChunkStreamer chama enqueue(key)
//   2. Worker thread gera blocos via TerrainGenerator
//   3. Resultado vai para outputQueue (thread-safe)
//   4. Loop principal chama flushReady() e cria os chunks no World
// ─────────────────────────────────────────────────────────────────────────────
class AsyncLoader {
public:
    struct ReadyChunk {
        glm::ivec3             key;
        std::vector<BlockType> blocks;  // SIZE_X × SIZE_Y × SIZE_Z elementos
    };

    explicit AsyncLoader(const TerrainGenerator& gen, int workerCount = 2);
    ~AsyncLoader();

    // Enfileira um chunk para geração em background
    void enqueue(glm::ivec3 chunkKey);

    // Retorna até maxPerFrame chunks prontos (chame na thread principal)
    std::vector<ReadyChunk> flushReady(int maxPerFrame = 4);

    size_t pendingCount() const { return pending.load(); }

    void shutdown();

private:
    const TerrainGenerator& gen;

    std::mutex              inputMutex;
    std::queue<glm::ivec3>  inputQueue;
    std::condition_variable inputCV;

    std::mutex             outputMutex;
    std::queue<ReadyChunk> outputQueue;

    std::vector<std::thread> workers;
    std::atomic<bool>        running { true };
    std::atomic<size_t>      pending { 0 };

    void workerLoop();
};

} // namespace fractal_engine::world