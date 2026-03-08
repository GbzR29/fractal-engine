#pragma once
#include "fractal_engine/world/terrain/Chunk.h"
#include "fractal_engine/world/terrain/TerrainGenerator.h"
#include "fractal_engine/world/BlockType.h"
#include "fractal_engine/graphics/Shader.h"
#include <third_party/glm/glm.hpp>
#include <unordered_map>
#include <functional>
#include <memory>

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
    // seed=0 → seed aleatória automática
    World(unsigned int seed = 0);

    ChunkMap&       getChunks()       { return chunks; }
    const ChunkMap& getChunks() const { return chunks; }

    void generateWorld(int radiusX, int radiusZ, Shader& shader);

    // ── Gerenciamento dinâmico de chunks (usado pelo WorldManager) ────────
    // addChunk: cria e inicializa o chunk se ainda não existir
    // removeChunk: descarrega o chunk da memória
    void addChunk   (glm::ivec3 key, Shader& shader);
    void removeChunk(glm::ivec3 key);

    // ── Render (duas passagens) ───────────────────────────────────────────
    //   renderOpaque:      blocos sólidos (com face culling)
    //   renderTransparent: folhas e água  (sem face culling)
    void renderOpaque     (Shader& shader);
    void renderTransparent(Shader& shader);
    // Mantido por compatibilidade — chama ambas as passagens
    void render(Shader& shader);

    // ── Queries ───────────────────────────────────────────────────────────
    glm::ivec3 getChunkKey  (int wx, int wy, int wz)       const;
    bool       isAirWorld   (int wx, int wy, int wz)       const;
    bool       isBlockSolid (float wx, float wy, float wz) const;
    BlockType  getBlockAt   (int wx, int wy, int wz)       const;
    float      getTerrainHeightAt(float worldX, float worldZ,
                                  float maxHeight = (float)(Chunk::SIZE_Y - 1)) const;

    // ── Luz ───────────────────────────────────────────────────────────────
    int   getWorldSkyLight  (int wx, int wy, int wz) const;
    int   getWorldBlockLight(int wx, int wy, int wz) const;
    float getWorldLightValue(int wx, int wy, int wz) const;

    // ── Edição ────────────────────────────────────────────────────────────
    bool breakBlock(glm::ivec3 pos);
    bool placeBlock(glm::ivec3 pos, BlockType blockType = BLOCK_STONE);

    unsigned int getSeed() const { return terrainSeed; }

    // Acesso ao gerador (para Player::initializeAtTerrainHeight etc.)
    const TerrainGenerator& getGenerator() const { return *generator; }

private:
    ChunkMap     chunks;
    unsigned int terrainSeed = 0;
    std::unique_ptr<TerrainGenerator> generator;

    void plantTrees        (Shader& shader);
    void plantTreeAt       (int wx, int wz, int trunkH, int crownR);
    void setBlockWorld     (int wx, int wy, int wz, BlockType type);
    void remeshChunk       (glm::ivec3 key);
    void remeshNeighbors   (glm::ivec3 key);
    void relightChunk      (glm::ivec3 key);
    void relightWorld      ();
};

} // namespace fractal_engine::world