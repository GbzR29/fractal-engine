#pragma once
#include "fractal_engine/world/terrain/Chunk.h"
#include "fractal_engine/world/BlockType.h"
#include "fractal_engine/graphics/Shader.h"
#include <third_party/glm/glm.hpp>
#include <unordered_map>
#include <functional>

namespace fractal_engine::world {

using fractal_engine::graphics::Shader;

// ─────────────────────────────────────────────────────────────────────────────
// Hash/Equal para glm::ivec3 — necessário para unordered_map
// Migrado de std::map + IVec3Cmp para unordered_map: O(1) vs O(log n) no lookup
// ─────────────────────────────────────────────────────────────────────────────
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

// ─────────────────────────────────────────────────────────────────────────────
// World — mapa de todos os chunks carregados, indexados pelo canto inferior
// ─────────────────────────────────────────────────────────────────────────────
class World {
public:
    // Acesso direto ao mapa (WorldManager e ChunkStreamer iteram aqui)
    ChunkMap& getChunks()             { return chunks; }
    const ChunkMap& getChunks() const { return chunks; }

    // ── Ciclo de vida dos chunks ───────────────────────────────────────────
    // Adiciona chunk em pos (ignora se já existe) e remesheia vizinhos
    void addChunk   (glm::ivec3 pos, Shader& shader);
    // Remove e destrói o chunk (libera VAO/VBO via destrutor de Chunk)
    void removeChunk(glm::ivec3 pos);

    // Gera um grid inicial em 2 passagens:
    //   1ª: cria todos os chunks e seus blocos
    //   2ª: gera as meshes COM vizinhos (sem costuras nas bordas)
    void generateWorld(int radiusX, int radiusZ, Shader& shader);

    // ── Render ────────────────────────────────────────────────────────────
    void render(Shader& shader);

    // ── Queries ───────────────────────────────────────────────────────────
    glm::ivec3 getChunkKey   (int wx, int wy, int wz)              const;
    bool       isAirWorld    (int wx, int wy, int wz)              const;
    bool       isBlockSolid  (float wx, float wy, float wz)        const;

    // Varre de cima para baixo e retorna y+1 do primeiro bloco sólido encontrado
    // maxHeight padrão = SIZE_Y - 1 (garante que começa acima de qualquer superfície)
    float getTerrainHeightAt(float worldX, float worldZ,
                              float maxHeight = (float)(Chunk::SIZE_Y - 1)) const;

    // ── Edição de blocos ──────────────────────────────────────────────────
    bool breakBlock(glm::ivec3 pos);
    bool placeBlock(glm::ivec3 pos, BlockType blockType = BLOCK_STONE);

private:
    ChunkMap chunks;

    void remeshChunk   (glm::ivec3 key);
    void remeshNeighbors(glm::ivec3 key);
};

} // namespace fractal_engine::world