#include "fractal_engine/world/World.h"
#include <cmath>
#include <iostream>
#include <chrono>
#include <memory>

namespace fractal_engine::world {

// ─────────────────────────────────────────────────────────────────────────────
// Construtor — seed=0 gera seed aleatória
// ─────────────────────────────────────────────────────────────────────────────
World::World(unsigned int seed) {
    if (seed == 0) {
        auto now = std::chrono::high_resolution_clock::now();
        terrainSeed = (unsigned int)now.time_since_epoch().count();
    } else {
        terrainSeed = seed;
    }
    generator = std::make_unique<TerrainGenerator>(terrainSeed);
    std::cout << "[World] Seed: " << terrainSeed << "\n";
}

// ─────────────────────────────────────────────────────────────────────────────
// Helpers de coordenada
// ─────────────────────────────────────────────────────────────────────────────
glm::ivec3 World::getChunkKey(int wx, int /*wy*/, int wz) const {
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
    int lx = wx - key.x, ly = wy, lz = wz - key.z;
    if (lx < 0 || lx >= Chunk::SIZE_X ||
        ly < 0 || ly >= Chunk::SIZE_Y ||
        lz < 0 || lz >= Chunk::SIZE_Z) return true;
    return it->second.isAir(lx, ly, lz);
}

bool World::isBlockSolid(float wx, float wy, float wz) const {
    return !isAirWorld((int)std::floor(wx), (int)std::floor(wy), (int)std::floor(wz));
}

BlockType World::getBlockAt(int wx, int wy, int wz) const {
    glm::ivec3 key = getChunkKey(wx, wy, wz);
    auto it = chunks.find(key);
    if (it == chunks.end()) return BLOCK_AIR;
    int lx = wx - key.x, ly = wy, lz = wz - key.z;
    if (lx < 0 || lx >= Chunk::SIZE_X ||
        ly < 0 || ly >= Chunk::SIZE_Y ||
        lz < 0 || lz >= Chunk::SIZE_Z) return BLOCK_AIR;
    return it->second.getBlock(lx, ly, lz);
}

void World::setBlockWorld(int wx, int wy, int wz, BlockType type) {
    if (wy < 0 || wy >= Chunk::SIZE_Y) return;
    glm::ivec3 key = getChunkKey(wx, wy, wz);
    auto it = chunks.find(key);
    if (it == chunks.end()) return;
    int lx = wx - key.x, lz = wz - key.z;
    if (lx < 0 || lx >= Chunk::SIZE_X ||
        lz < 0 || lz >= Chunk::SIZE_Z) return;
    it->second.setBlock(lx, wy, lz, type);
}

// ─────────────────────────────────────────────────────────────────────────────
// Luz
// ─────────────────────────────────────────────────────────────────────────────
int World::getWorldSkyLight(int wx, int wy, int wz) const {
    glm::ivec3 key = getChunkKey(wx, wy, wz);
    auto it = chunks.find(key);
    if (it == chunks.end()) return Chunk::SKY_LIGHT_MAX;
    int lx = wx - key.x, ly = wy, lz = wz - key.z;
    return it->second.getSkyLight(lx, ly, lz);
}

int World::getWorldBlockLight(int wx, int wy, int wz) const {
    glm::ivec3 key = getChunkKey(wx, wy, wz);
    auto it = chunks.find(key);
    if (it == chunks.end()) return 0;
    int lx = wx - key.x, ly = wy, lz = wz - key.z;
    return it->second.getBlockLight(lx, ly, lz);
}

float World::getWorldLightValue(int wx, int wy, int wz) const {
    int sky   = getWorldSkyLight  (wx, wy, wz);
    int block = getWorldBlockLight(wx, wy, wz);
    int best  = sky > block ? sky : block;
    return (float)best / (float)Chunk::MAX_LIGHT;
}

// ─────────────────────────────────────────────────────────────────────────────
// relightChunk / relightWorld
// ─────────────────────────────────────────────────────────────────────────────
void World::relightChunk(glm::ivec3 key) {
    auto it = chunks.find(key);
    if (it == chunks.end()) return;
    Chunk& chunk = it->second;
    chunk.clearLight();
    chunk.initSkyLight();

    auto worldGetBlock = [this](int wx, int wy, int wz) -> BlockType {
        return getBlockAt(wx, wy, wz);
    };
    auto worldGetSkyLight = [this](int wx, int wy, int wz) -> int {
        return getWorldSkyLight(wx, wy, wz);
    };
    chunk.propagateLight(worldGetBlock, worldGetSkyLight);
}

void World::relightWorld() {
    for (auto& [key, chunk] : chunks) {
        chunk.clearLight();
        chunk.initSkyLight();
    }
    auto worldGetBlock = [this](int wx, int wy, int wz) -> BlockType {
        return getBlockAt(wx, wy, wz);
    };
    auto worldGetSkyLight = [this](int wx, int wy, int wz) -> int {
        return getWorldSkyLight(wx, wy, wz);
    };
    for (auto& [key, chunk] : chunks)
        chunk.propagateLight(worldGetBlock, worldGetSkyLight);
}

// ─────────────────────────────────────────────────────────────────────────────
// remeshChunk / remeshNeighbors
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
        { Chunk::SIZE_X, 0,  0}, {-Chunk::SIZE_X, 0,  0},
        {0, 0,  Chunk::SIZE_Z}, {0, 0, -Chunk::SIZE_Z},
    };
    for (auto& off : offsets) {
        glm::ivec3 nk = key + off;
        if (chunks.count(nk)) remeshChunk(nk);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// plantTreeAt
// ─────────────────────────────────────────────────────────────────────────────
void World::plantTreeAt(int wx, int wz, int trunkH, int crownR) {
    int groundY = -1;
    for (int y = Chunk::SIZE_Y - 2; y >= 1; y--) {
        BlockType bt = getBlockAt(wx, y, wz);
        if (bt == BLOCK_GRASS || bt == BLOCK_DIRT || bt == BLOCK_SAND) {
            groundY = y;
            break;
        }
    }
    if (groundY < 0) return;

    for (int i = 1; i <= trunkH; i++)
        setBlockWorld(wx, groundY + i, wz, BLOCK_WOOD);

    int topY = groundY + trunkH;
    for (int dy = -crownR; dy <= crownR + 1; dy++)
    for (int dx = -crownR; dx <= crownR;     dx++)
    for (int dz = -crownR; dz <= crownR;     dz++) {
        float dist = std::sqrt((float)(dx*dx + dy*dy*0.7f + dz*dz));
        if (dist > (float)crownR + 0.5f) continue;
        int bx = wx + dx, by = topY + dy, bz = wz + dz;
        if (by <= groundY + trunkH - 1) continue;
        if (getBlockAt(bx, by, bz) == BLOCK_AIR)
            setBlockWorld(bx, by, bz, BLOCK_LEAF);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// plantTrees — percorre todos os chunks e planta árvores
// ─────────────────────────────────────────────────────────────────────────────
void World::plantTrees(Shader& /*shader*/) {
    for (auto& [key, chunk] : chunks) {
        int cx = (int)chunk.position.x;
        int cz = (int)chunk.position.z;
        auto trees = generator->getTreesForChunk(cx, cz, Chunk::SIZE_X, Chunk::SIZE_Z);
        for (auto& tree : trees)
            plantTreeAt(tree.wx, tree.wz, tree.trunkHeight, tree.crownRadius);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// addChunk — cria e inicializa um chunk se ainda não existir
// ─────────────────────────────────────────────────────────────────────────────
void World::addChunk(glm::ivec3 key, Shader& shader) {
    // Ignora se o chunk já existe
    if (chunks.count(key)) return;

    // Cria o chunk e gera blocos
    auto& chunk = chunks.emplace(
        std::piecewise_construct,
        std::forward_as_tuple(key),
        std::forward_as_tuple(glm::vec3(key), shader)
    ).first->second;

    chunk.generateBlocks(*generator);

    // Planta árvores somente neste chunk
    auto trees = generator->getTreesForChunk(
        key.x, key.z, Chunk::SIZE_X, Chunk::SIZE_Z);
    for (auto& tree : trees)
        plantTreeAt(tree.wx, tree.wz, tree.trunkHeight, tree.crownRadius);

    // Luz
    relightChunk(key);

    // Mesh
    auto worldIsAir    = [this](int wx, int wy, int wz) { return isAirWorld(wx, wy, wz); };
    auto worldGetLight = [this](int wx, int wy, int wz) { return getWorldLightValue(wx, wy, wz); };
    chunk.generateMesh(worldIsAir, worldGetLight);
    chunk.uploadMesh();

    // Fecha costuras visuais nos chunks vizinhos
    remeshNeighbors(key);
}

// ─────────────────────────────────────────────────────────────────────────────
// removeChunk — descarrega um chunk da memória
// ─────────────────────────────────────────────────────────────────────────────
void World::removeChunk(glm::ivec3 key) {
    auto it = chunks.find(key);
    if (it == chunks.end()) return;

    chunks.erase(it);

    // Remesh dos vizinhos para não deixar faces abertas
    remeshNeighbors(key);
}

// ─────────────────────────────────────────────────────────────────────────────
// generateWorld — 4 passagens: blocos → árvores → luz → mesh
// ─────────────────────────────────────────────────────────────────────────────
void World::generateWorld(int radiusX, int radiusZ, Shader& shader) {
    // 1ª: cria chunks e gera blocos base
    for (int cx = -radiusX; cx <= radiusX; cx++)
    for (int cz = -radiusZ; cz <= radiusZ; cz++) {
        glm::ivec3 pos(cx * Chunk::SIZE_X, 0, cz * Chunk::SIZE_Z);
        if (!chunks.count(pos)) {
            auto& chunk = chunks.emplace(std::piecewise_construct,
                           std::forward_as_tuple(pos),
                           std::forward_as_tuple(glm::vec3(pos), shader)).first->second;
            chunk.generateBlocks(*generator);
        }
    }

    // 2ª: planta árvores (precisa de todos os chunks gerados)
    plantTrees(shader);

    // 3ª: calcula luz
    relightWorld();

    // 4ª: gera meshes
    auto worldIsAir    = [this](int wx, int wy, int wz) { return isAirWorld(wx, wy, wz); };
    auto worldGetLight = [this](int wx, int wy, int wz) { return getWorldLightValue(wx, wy, wz); };
    for (auto& [key, chunk] : chunks) {
        chunk.generateMesh(worldIsAir, worldGetLight);
        chunk.uploadMesh();
    }

    std::cout << "[World] " << chunks.size() << " chunks | seed " << terrainSeed << "\n";
}

// ─────────────────────────────────────────────────────────────────────────────
// render — duas passagens
// ─────────────────────────────────────────────────────────────────────────────
void World::renderOpaque(Shader& shader) {
    glEnable(GL_CULL_FACE);
    for (auto& [key, chunk] : chunks)
        chunk.DrawOpaque(shader);
}

void World::renderTransparent(Shader& shader) {
    glDisable(GL_CULL_FACE);
    for (auto& [key, chunk] : chunks)
        chunk.DrawTransparent(shader);
    glEnable(GL_CULL_FACE);
}

void World::render(Shader& shader) {
    renderOpaque(shader);
    renderTransparent(shader);
}

// ─────────────────────────────────────────────────────────────────────────────
// getTerrainHeightAt
// ─────────────────────────────────────────────────────────────────────────────
float World::getTerrainHeightAt(float worldX, float worldZ, float maxHeight) const {
    for (int y = (int)maxHeight; y >= 0; y--) {
        if (isBlockSolid(worldX, (float)y, worldZ))
            return (float)(y + 1);
    }
    return (float)(Chunk::SIZE_Y / 2);
}

// ─────────────────────────────────────────────────────────────────────────────
// breakBlock / placeBlock
// ─────────────────────────────────────────────────────────────────────────────
bool World::breakBlock(glm::ivec3 pos) {
    glm::ivec3 key = getChunkKey(pos.x, pos.y, pos.z);
    auto it = chunks.find(key);
    if (it == chunks.end()) return false;
    int lx = pos.x - key.x, ly = pos.y, lz = pos.z - key.z;
    if (lx < 0 || lx >= Chunk::SIZE_X ||
        ly < 0 || ly >= Chunk::SIZE_Y ||
        lz < 0 || lz >= Chunk::SIZE_Z) return false;
    it->second.setBlock(lx, ly, lz, BLOCK_AIR);
    relightChunk(key);
    remeshNeighbors(key);
    remeshChunk(key);
    return true;
}

bool World::placeBlock(glm::ivec3 pos, BlockType blockType) {
    if (!isAirWorld(pos.x, pos.y, pos.z)) return false;
    glm::ivec3 key = getChunkKey(pos.x, pos.y, pos.z);
    auto it = chunks.find(key);
    if (it == chunks.end()) return false;
    int lx = pos.x - key.x, ly = pos.y, lz = pos.z - key.z;
    if (lx < 0 || lx >= Chunk::SIZE_X ||
        ly < 0 || ly >= Chunk::SIZE_Y ||
        lz < 0 || lz >= Chunk::SIZE_Z) return false;
    it->second.setBlock(lx, ly, lz, blockType);
    relightChunk(key);
    remeshNeighbors(key);
    remeshChunk(key);
    return true;
}

} // namespace fractal_engine::world