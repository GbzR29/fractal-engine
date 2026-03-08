#include "fractal_engine/world/World.h"
#include <cmath>
#include <limits>
#include <iostream>

namespace fractal_engine::world {

// ─────────────────────────────────────────────────────────────────────────────
// Helpers de coordenada
// ─────────────────────────────────────────────────────────────────────────────
glm::ivec3 World::getChunkKey(int wx, int wy, int wz) const {
    auto floorDiv = [](int a, int b) -> int {
        return a / b - (a % b != 0 && (a ^ b) < 0);
    };
    return glm::ivec3(
        floorDiv(wx, Chunk::SIZE_X) * Chunk::SIZE_X,
        0,
        floorDiv(wz, Chunk::SIZE_Z) * Chunk::SIZE_Z
    );
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
        lz < 0 || lz >= Chunk::SIZE_Z)
        return true;

    return it->second.isAir(lx, ly, lz);
}

bool World::isBlockSolid(float wx, float wy, float wz) const {
    return !isAirWorld((int)std::floor(wx), (int)std::floor(wy), (int)std::floor(wz));
}

// ─────────────────────────────────────────────────────────────────────────────
// Luz — getters mundo
// ─────────────────────────────────────────────────────────────────────────────
int World::getWorldSkyLight(int wx, int wy, int wz) const {
    glm::ivec3 key = getChunkKey(wx, wy, wz);
    auto it = chunks.find(key);
    if (it == chunks.end()) return Chunk::SKY_LIGHT_MAX; // fora do mundo = céu

    int lx = wx - key.x;
    int ly = wy - key.y;
    int lz = wz - key.z;
    return it->second.getSkyLight(lx, ly, lz);
}

int World::getWorldBlockLight(int wx, int wy, int wz) const {
    glm::ivec3 key = getChunkKey(wx, wy, wz);
    auto it = chunks.find(key);
    if (it == chunks.end()) return 0;

    int lx = wx - key.x;
    int ly = wy - key.y;
    int lz = wz - key.z;
    return it->second.getBlockLight(lx, ly, lz);
}

float World::getWorldLightValue(int wx, int wy, int wz) const {
    int sky   = getWorldSkyLight  (wx, wy, wz);
    int block = getWorldBlockLight(wx, wy, wz);
    int best  = sky > block ? sky : block;
    return (float)best / (float)Chunk::MAX_LIGHT;
}

// ─────────────────────────────────────────────────────────────────────────────
// relightChunk
//
// Recalcula luz de um chunk inteiro:
//   1. Zera lightMap
//   2. initSkyLight (propaga verticalmente dentro do chunk)
//   3. propagateLight BFS (difunde para os 6 vizinhos dentro do chunk)
//      com acesso ao blocklight dos chunks vizinhos via worldGetBlockLight
//
// ─────────────────────────────────────────────────────────────────────────────
void World::relightChunk(glm::ivec3 key) {
    auto it = chunks.find(key);
    if (it == chunks.end()) return;

    Chunk& chunk = it->second;
    chunk.clearLight();
    chunk.initSkyLight();

    // Passa skylight dos vizinhos para o BFS poder injetar luz pelas bordas
    auto worldGetBlock = [this](int wx, int wy, int wz) -> BlockType {
        glm::ivec3 k = getChunkKey(wx, wy, wz);
        auto jt = chunks.find(k);
        if (jt == chunks.end()) return BLOCK_AIR;
        int lx = wx - k.x, ly = wy - k.y, lz = wz - k.z;
        return jt->second.getBlock(lx, ly, lz);
    };

    auto worldGetSkyLight = [this](int wx, int wy, int wz) -> int {
        return getWorldSkyLight(wx, wy, wz);
    };

    chunk.propagateLight(worldGetBlock, worldGetSkyLight);
}

// ─────────────────────────────────────────────────────────────────────────────
// relightWorld
//
// Recalcula luz de todos os chunks em 2 passagens:
//   Passagem 1: skylight (initSkyLight em todos)
//   Passagem 2: propagação BFS (com acesso cross-chunk)
//
// Duas passagens garantem que a luz dos vizinhos já está disponível
// para o BFS de cada chunk.
// ─────────────────────────────────────────────────────────────────────────────
void World::relightWorld() {
    // Passagem 1: skylight vertical em todos os chunks
    for (auto& [key, chunk] : chunks) {
        chunk.clearLight();
        chunk.initSkyLight();
    }

    // Passagem 2: BFS completo com acesso cross-chunk
    auto worldGetBlock = [this](int wx, int wy, int wz) -> BlockType {
        glm::ivec3 k = getChunkKey(wx, wy, wz);
        auto it = chunks.find(k);
        if (it == chunks.end()) return BLOCK_AIR;
        int lx = wx - k.x, ly = wy - k.y, lz = wz - k.z;
        return it->second.getBlock(lx, ly, lz);
    };

    auto worldGetSkyLight = [this](int wx, int wy, int wz) -> int {
        return getWorldSkyLight(wx, wy, wz);
    };

    for (auto& [key, chunk] : chunks) {
        chunk.propagateLight(worldGetBlock, worldGetSkyLight);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// remeshChunk — agora passa worldGetLight para iluminar bordas corretamente
// ─────────────────────────────────────────────────────────────────────────────
void World::remeshChunk(glm::ivec3 key) {
    auto it = chunks.find(key);
    if (it == chunks.end()) return;

    auto worldIsAir = [this](int wx, int wy, int wz) -> bool {
        return isAirWorld(wx, wy, wz);
    };

    auto worldGetLight = [this](int wx, int wy, int wz) -> float {
        return getWorldLightValue(wx, wy, wz);
    };

    it->second.generateMesh(worldIsAir, worldGetLight);
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
        glm::ivec3 nk = key + off;
        if (chunks.count(nk))
            remeshChunk(nk);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// generateWorld — 3 passagens: blocos → luz → mesh
// ─────────────────────────────────────────────────────────────────────────────
void World::generateWorld(int radiusX, int radiusZ, Shader& shader) {
    // 1ª passagem: cria todos os chunks e gera blocos
    for (int cx = -radiusX; cx <= radiusX; cx++)
    for (int cz = -radiusZ; cz <= radiusZ; cz++) {
        glm::ivec3 pos(cx * Chunk::SIZE_X, 0, cz * Chunk::SIZE_Z);
        if (!chunks.count(pos)) {
            chunks.emplace(std::piecewise_construct,
                           std::forward_as_tuple(pos),
                           std::forward_as_tuple(glm::vec3(pos), shader));
        }
    }

    // 2ª passagem: calcula luz (skylight + BFS) para todo o mundo
    relightWorld();

    // 3ª passagem: gera meshes com luz correta e vizinhos disponíveis
    auto worldIsAir = [this](int wx, int wy, int wz) -> bool {
        return isAirWorld(wx, wy, wz);
    };
    auto worldGetLight = [this](int wx, int wy, int wz) -> float {
        return getWorldLightValue(wx, wy, wz);
    };

    for (auto& [key, chunk] : chunks) {
        chunk.generateMesh(worldIsAir, worldGetLight);
        chunk.uploadMesh();
    }

    std::cout << "[World] " << chunks.size() << " chunks gerados com iluminação.\n";
}

// ─────────────────────────────────────────────────────────────────────────────
// addChunk
// ─────────────────────────────────────────────────────────────────────────────
void World::addChunk(glm::ivec3 pos, Shader& shader) {
    if (chunks.count(pos)) return;

    chunks.emplace(std::piecewise_construct,
                   std::forward_as_tuple(pos),
                   std::forward_as_tuple(glm::vec3(pos), shader));

    relightChunk(pos);
    remeshChunk(pos);
    remeshNeighbors(pos);
}

// ─────────────────────────────────────────────────────────────────────────────
// Altura do terreno
// ─────────────────────────────────────────────────────────────────────────────
float World::getTerrainHeightAt(float worldX, float worldZ, float maxHeight) const {
    for (int y = (int)maxHeight; y >= 0; y--) {
        if (isBlockSolid(worldX, (float)y, worldZ))
            return (float)(y + 1);
    }
    return (float)(Chunk::SIZE_Y / 2);
}

// ─────────────────────────────────────────────────────────────────────────────
// render
// ─────────────────────────────────────────────────────────────────────────────
void World::render(Shader& shader) {
    for (auto& [key, chunk] : chunks)
        chunk.Draw(shader);
}

// ─────────────────────────────────────────────────────────────────────────────
// breakBlock / placeBlock — recalcula luz após mudança
// ─────────────────────────────────────────────────────────────────────────────
bool World::breakBlock(glm::ivec3 pos) {
    glm::ivec3 key = getChunkKey(pos.x, pos.y, pos.z);
    auto it = chunks.find(key);
    if (it == chunks.end()) return false;

    int lx = pos.x - key.x, ly = pos.y - key.y, lz = pos.z - key.z;
    if (lx < 0 || lx >= Chunk::SIZE_X ||
        ly < 0 || ly >= Chunk::SIZE_Y ||
        lz < 0 || lz >= Chunk::SIZE_Z) return false;

    it->second.setBlock(lx, ly, lz, BLOCK_AIR);

    // Recalcula luz do chunk afetado e vizinhos, depois remesh
    relightChunk(key);
    remeshNeighbors(key);   // vizinhos podem ter bordas iluminadas agora
    remeshChunk(key);
    return true;
}

bool World::placeBlock(glm::ivec3 pos, BlockType blockType) {
    if (!isAirWorld(pos.x, pos.y, pos.z)) return false;

    glm::ivec3 key = getChunkKey(pos.x, pos.y, pos.z);
    auto it = chunks.find(key);
    if (it == chunks.end()) return false;

    int lx = pos.x - key.x, ly = pos.y - key.y, lz = pos.z - key.z;
    if (lx < 0 || lx >= Chunk::SIZE_X ||
        ly < 0 || ly >= Chunk::SIZE_Y ||
        lz < 0 || lz >= Chunk::SIZE_Z) return false;

    it->second.setBlock(lx, ly, lz, blockType);

    // Bloco novo pode bloquear luz — recalcula
    relightChunk(key);
    remeshNeighbors(key);
    remeshChunk(key);
    return true;
}

} // namespace fractal_engine::world