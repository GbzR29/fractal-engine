#include "fractal_engine/world/World.h"
#include <cmath>

namespace fractal_engine::world {

// ─────────────────────────────────────────────
// Helpers de coordenada
// ─────────────────────────────────────────────
glm::ivec3 World::getChunkKey(int wx, int wy, int wz) const {
    // Arredonda para o múltiplo de SIZE mais próximo (em direção a -inf)
    auto floorDiv = [](int a, int b) -> int {
        return a / b - (a % b != 0 && (a ^ b) < 0);
    };
    int cx = floorDiv(wx, Chunk::SIZE_X) * Chunk::SIZE_X;
    int cy = 0; // Chunks não se dividem em Y (apenas um chunk vertical por coluna)
    int cz = floorDiv(wz, Chunk::SIZE_Z) * Chunk::SIZE_Z;
    return glm::ivec3(cx, cy, cz);
}

bool World::isAirWorld(int wx, int wy, int wz) const {
    glm::ivec3 key = getChunkKey(wx, wy, wz);
    auto it = chunks.find(key);
    if (it == chunks.end()) return true; // chunk não carregado = ar

    int lx = wx - key.x;
    int ly = wy - key.y;
    int lz = wz - key.z;
    return it->second.isAir(lx, ly, lz);
}

bool World::isBlockSolid(float wx, float wy, float wz) const {
    return !isAirWorld((int)std::floor(wx), (int)std::floor(wy), (int)std::floor(wz));
}

// ─────────────────────────────────────────────
// Remesh de um chunk (com callback de vizinhos)
// ─────────────────────────────────────────────
void World::remeshChunk(glm::ivec3 key) {
    auto it = chunks.find(key);
    if (it == chunks.end()) return;

    // Callback que o Chunk usa para checar blocos fora dos seus limites
    auto worldIsAir = [this](int wx, int wy, int wz) -> bool {
        return isAirWorld(wx, wy, wz);
    };

    it->second.generateMesh(worldIsAir);
    it->second.uploadMesh();
}

// Atualiza os 4 vizinhos horizontais de um chunk para corrigir costuras
void World::remeshNeighbors(glm::ivec3 key) {
    const glm::ivec3 offsets[] = {
        { Chunk::SIZE_X, 0,  0},
        {-Chunk::SIZE_X, 0,  0},
        {0, 0,  Chunk::SIZE_Z},
        {0, 0, -Chunk::SIZE_Z},
    };
    for (auto& off : offsets) {
        glm::ivec3 neighborKey = key + off;
        if (chunks.count(neighborKey)) {
            remeshChunk(neighborKey);
        }
    }
}

// ─────────────────────────────────────────────
// Adicionar chunk
// ─────────────────────────────────────────────
void World::addChunk(glm::ivec3 pos, Shader& shader) {
    if (chunks.count(pos)) return; // já existe

    // Insere o chunk (construtor já chama generateBlocks + generateMesh inicial)
    chunks.emplace(std::piecewise_construct,
                   std::forward_as_tuple(pos),
                   std::forward_as_tuple(glm::vec3(pos), shader));

    // Agora que o chunk existe e seus vizinhos também, refaz a mesh
    // do novo chunk e dos vizinhos para eliminar faces expostas incorretamente
    remeshChunk(pos);
    remeshNeighbors(pos);
}

// ─────────────────────────────────────────────
// Gerar mundo em grid
// ─────────────────────────────────────────────
void World::generateWorld(int radiusX, int radiusZ, Shader& shader) {
    // 1ª passagem: gera todos os blocos
    for (int cx = -radiusX; cx <= radiusX; cx++) {
        for (int cz = -radiusZ; cz <= radiusZ; cz++) {
            glm::ivec3 pos(cx * Chunk::SIZE_X, 0, cz * Chunk::SIZE_Z);
            if (!chunks.count(pos)) {
                chunks.emplace(std::piecewise_construct,
                               std::forward_as_tuple(pos),
                               std::forward_as_tuple(glm::vec3(pos), shader));
            }
        }
    }

    // 2ª passagem: gera meshes COM informação de vizinhos (sem costuras!)
    auto worldIsAir = [this](int wx, int wy, int wz) -> bool {
        return isAirWorld(wx, wy, wz);
    };

    for (auto& [key, chunk] : chunks) {
        chunk.generateMesh(worldIsAir);
        chunk.uploadMesh();
    }
}

// ─────────────────────────────────────────────
// Render
// ─────────────────────────────────────────────
void World::render(Shader& shader) {
    for (auto& [key, chunk] : chunks) {
        chunk.Draw(shader);
    }
}

} // namespace fractal_engine::world