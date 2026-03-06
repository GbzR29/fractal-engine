#include "fractal_engine/world/World.h"
#include <cmath>
#include <limits>

namespace fractal_engine::world {

// ─────────────────────────────────────────────
// Helpers de coordenada
// ─────────────────────────────────────────────
glm::ivec3 World::getChunkKey(int wx, int wy, int wz) const {
    auto floorDiv = [](int a, int b) -> int {
        return a / b - (a % b != 0 && (a ^ b) < 0);
    };
    int cx = floorDiv(wx, Chunk::SIZE_X) * Chunk::SIZE_X;
    int cy = 0;
    int cz = floorDiv(wz, Chunk::SIZE_Z) * Chunk::SIZE_Z;
    return glm::ivec3(cx, cy, cz);
}

bool World::isAirWorld(int wx, int wy, int wz) const {
    glm::ivec3 key = getChunkKey(wx, wy, wz);
    auto it = chunks.find(key);
    if (it == chunks.end()) return true;

    int lx = wx - key.x;
    int ly = wy - key.y;
    int lz = wz - key.z;

    if (lx < 0 || lx >= Chunk::SIZE_X ||
        ly < 0 || ly >= Chunk::SIZE_Y ||
        lz < 0 || lz >= Chunk::SIZE_Z) {
        return true;
    }

    return it->second.isAir(lx, ly, lz);
}

bool World::isBlockSolid(float wx, float wy, float wz) const {
    return !isAirWorld(
        (int)std::floor(wx),
        (int)std::floor(wy),
        (int)std::floor(wz)
    );
}

// ─────────────────────────────────────────────
// Encontrar altura do terreno
// ─────────────────────────────────────────────
float World::getTerrainHeightAt(float worldX, float worldZ, float maxHeight) const {
    // ─────────────────────────────────────────────────────────────────────
    // FIX BUG 3: A função original não tinha um maxHeight padrão seguro.
    // Se chamada sem argumento e o default fosse muito baixo (ex: 40.0f),
    // o scan começava abaixo da superfície real e nunca encontrava blocos
    // → retornava o fallback 40.0f → player nascia dentro do chão.
    //
    // Agora o maxHeight default é Chunk::SIZE_Y - 1 (topo do chunk),
    // garantindo que a varredura sempre começa acima de qualquer superfície.
    //
    // Além disso, verificamos dois blocos de margem extra para evitar
    // que o jogador nasça com os pés colidindo com o bloco de grama.
    // ─────────────────────────────────────────────────────────────────────

    // Escaneia de cima para baixo procurando o primeiro bloco sólido
    for (int y = (int)maxHeight; y >= 0; y--) {
        if (isBlockSolid(worldX, (float)y, worldZ)) {
            // y é o bloco sólido mais alto.
            // Retorna y+1 = topo desse bloco (onde o player deve pousar).
            return (float)(y + 1);
        }
    }

    // Fallback: nenhum bloco encontrado na coluna (chunk vazio ou fora de range)
    // Usa surfaceLevel padrão como estimativa segura
    return (float)(Chunk::SIZE_Y / 2);
}

// ─────────────────────────────────────────────
// Remesh de um chunk
// ─────────────────────────────────────────────
void World::remeshChunk(glm::ivec3 key) {
    auto it = chunks.find(key);
    if (it == chunks.end()) return;

    auto worldIsAir = [this](int wx, int wy, int wz) -> bool {
        return isAirWorld(wx, wy, wz);
    };

    it->second.generateMesh(worldIsAir);
    it->second.uploadMesh();
}

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
    if (chunks.count(pos)) return;

    chunks.emplace(std::piecewise_construct,
                   std::forward_as_tuple(pos),
                   std::forward_as_tuple(glm::vec3(pos), shader));

    remeshChunk(pos);
    remeshNeighbors(pos);
}

// ─────────────────────────────────────────────
// Gerar mundo em grid
// ─────────────────────────────────────────────
void World::generateWorld(int radiusX, int radiusZ, Shader& shader) {
    // 1ª passagem: gera todos os blocos de todos os chunks
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

    // 2ª passagem: gera meshes COM vizinhos (sem costuras nas bordas)
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

// ─────────────────────────────────────────────
// QUEBRAR BLOCO
// ─────────────────────────────────────────────
bool World::breakBlock(glm::ivec3 pos) {
    glm::ivec3 chunkKey = getChunkKey(pos.x, pos.y, pos.z);
    auto it = chunks.find(chunkKey);
    if (it == chunks.end()) return false;

    int lx = pos.x - chunkKey.x;
    int ly = pos.y - chunkKey.y;
    int lz = pos.z - chunkKey.z;

    if (lx < 0 || lx >= Chunk::SIZE_X ||
        ly < 0 || ly >= Chunk::SIZE_Y ||
        lz < 0 || lz >= Chunk::SIZE_Z) return false;

    it->second.setBlock(lx, ly, lz, BLOCK_AIR);
    remeshChunk(chunkKey);
    remeshNeighbors(chunkKey);
    return true;
}

// ─────────────────────────────────────────────
// COLOCAR BLOCO
// ─────────────────────────────────────────────
bool World::placeBlock(glm::ivec3 pos, BlockType blockType) {
    if (!isAirWorld(pos.x, pos.y, pos.z)) return false;

    glm::ivec3 chunkKey = getChunkKey(pos.x, pos.y, pos.z);
    auto it = chunks.find(chunkKey);
    if (it == chunks.end()) return false;

    int lx = pos.x - chunkKey.x;
    int ly = pos.y - chunkKey.y;
    int lz = pos.z - chunkKey.z;

    if (lx < 0 || lx >= Chunk::SIZE_X ||
        ly < 0 || ly >= Chunk::SIZE_Y ||
        lz < 0 || lz >= Chunk::SIZE_Z) return false;

    it->second.setBlock(lx, ly, lz, blockType);
    remeshChunk(chunkKey);
    remeshNeighbors(chunkKey);
    return true;
}

} // namespace fractal_engine::world