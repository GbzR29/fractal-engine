#pragma once
#include "fractal_engine/world/terrain/Chunk.h"
#include "fractal_engine/world/BlockType.h"
#include "fractal_engine/graphics/Shader.h"
#include <third_party/glm/glm.hpp>
#include <unordered_map>
#include <functional>

namespace fractal_engine::world {

using fractal_engine::graphics::Shader;

struct IVec3Hash {
    size_t operator()(const glm::ivec3& v) const {
        size_t h = std::hash<int>{}(v.x);
        h ^= std::hash<int>{}(v.y) * 2654435761ULL + 0x9e3779b9 + (h << 6) + (h >> 2);
        h ^= std::hash<int>{}(v.z) * 2246822519ULL + 0x9e3779b9 + (h << 6) + (h >> 2);
        return h;
    }
};

struct IVec3Equal {
    bool operator()(const glm::ivec3& a, const glm::ivec3& b) const {
        return a.x == b.x && a.y == b.y && a.z == b.z;
    }
};

using ChunkMap = std::unordered_map<glm::ivec3, Chunk, IVec3Hash, IVec3Equal>;

class World {
public:
    ChunkMap&       getChunks()       { return chunks; }
    const ChunkMap& getChunks() const { return chunks; }

    void addChunk   (glm::ivec3 pos, Shader& shader);
    void removeChunk(glm::ivec3 pos);
    void generateWorld(int radiusX, int radiusZ, Shader& shader);
    void render(Shader& shader);

    // ── Queries ───────────────────────────────────────────────────────────
    glm::ivec3 getChunkKey  (int wx, int wy, int wz)       const;
    bool       isAirWorld   (int wx, int wy, int wz)       const;
    bool       isBlockSolid (float wx, float wy, float wz) const;
    float      getTerrainHeightAt(float worldX, float worldZ,
                                  float maxHeight = (float)(Chunk::SIZE_Y - 1)) const;

    // ── Luz ───────────────────────────────────────────────────────────────
    // Retorna skylight de uma posição mundo
    int   getWorldSkyLight  (int wx, int wy, int wz) const;
    // Retorna blocklight de uma posição mundo
    int   getWorldBlockLight(int wx, int wy, int wz) const;
    // Retorna max(sky, block) / 15 como float [0,1]
    float getWorldLightValue(int wx, int wy, int wz) const;

    // ── Edição de blocos ──────────────────────────────────────────────────
    bool breakBlock(glm::ivec3 pos);
    bool placeBlock(glm::ivec3 pos, BlockType blockType = BLOCK_STONE);

private:
    ChunkMap chunks;

    void remeshChunk   (glm::ivec3 key);
    void remeshNeighbors(glm::ivec3 key);

    // Recalcula luz de um chunk e seus vizinhos
    void relightChunk  (glm::ivec3 key);
    void relightWorld  ();   // chamado após generateWorld
};

} // namespace fractal_engine::world