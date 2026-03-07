#include "fractal_engine/world/streaming/AsyncLoader.h"
#include "fractal_engine/world/terrain/Chunk.h"

namespace fractal_engine::world {

AsyncLoader::AsyncLoader(const TerrainGenerator& gen, int workerCount)
    : gen(gen)
{
    for (int i = 0; i < workerCount; i++) {
        workers.emplace_back([this] { workerLoop(); });
    }
}

AsyncLoader::~AsyncLoader() {
    shutdown();
}

// ─────────────────────────────────────────────
// Enfileira chunk para geração em background
// ─────────────────────────────────────────────
void AsyncLoader::enqueue(glm::ivec3 chunkKey) {
    {
        std::lock_guard<std::mutex> lock(inputMutex);
        inputQueue.push(chunkKey);
        pending++;
    }
    inputCV.notify_one();
}

// ─────────────────────────────────────────────
// Retorna chunks prontos (thread principal)
// ─────────────────────────────────────────────
std::vector<AsyncLoader::ReadyChunk> AsyncLoader::flushReady(int maxPerFrame) {
    std::vector<ReadyChunk> result;
    std::lock_guard<std::mutex> lock(outputMutex);

    for (int i = 0; i < maxPerFrame && !outputQueue.empty(); i++) {
        result.push_back(std::move(outputQueue.front()));
        outputQueue.pop();
    }

    return result;
}

// ─────────────────────────────────────────────
// Para todos os workers
// ─────────────────────────────────────────────
void AsyncLoader::shutdown() {
    running = false;
    inputCV.notify_all();
    for (auto& w : workers) {
        if (w.joinable()) w.join();
    }
    workers.clear();
}

// ─────────────────────────────────────────────
// Loop do worker — roda em thread separada
// NOTA: TerrainGenerator é thread-safe (só leitura após construção)
// ─────────────────────────────────────────────
void AsyncLoader::workerLoop() {
    while (running) {
        glm::ivec3 key;

        // Espera um item na fila de entrada
        {
            std::unique_lock<std::mutex> lock(inputMutex);
            inputCV.wait(lock, [this] {
                return !inputQueue.empty() || !running;
            });

            if (!running && inputQueue.empty()) break;

            key = inputQueue.front();
            inputQueue.pop();
        }

        // ── Gera blocos (sem OpenGL — só CPU) ─────────────────────────────
        const int total = Chunk::SIZE_X * Chunk::SIZE_Y * Chunk::SIZE_Z;
        ReadyChunk ready;
        ready.key    = key;
        ready.blocks.resize(total, BLOCK_AIR);

        gen.generateChunkBlocks(
            ready.blocks.data(),
            key.x, key.y, key.z,
            Chunk::SIZE_X, Chunk::SIZE_Y, Chunk::SIZE_Z
        );

        // ── Envia para fila de saída ───────────────────────────────────────
        {
            std::lock_guard<std::mutex> lock(outputMutex);
            outputQueue.push(std::move(ready));
        }

        pending--;
    }
}

} // namespace fractal_engine::world