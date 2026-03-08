#pragma once
#include "fractal_engine/math/Noise.h"
#include "fractal_engine/world/BlockType.h"
#include <vector>

namespace fractal_engine::world {

// ─────────────────────────────────────────────────────────────────────────────
// Biome — determina aparência e estruturas de uma região
// ─────────────────────────────────────────────────────────────────────────────
enum class Biome {
    Plains,    // grama, terreno suave
    Forest,    // grama, árvores densas
    Desert,    // areia, terreno plano
    Mountain,  // pedra exposta, picos altos
    Ocean,     // baixo, coberto de água
};

// ─────────────────────────────────────────────────────────────────────────────
// TreeData — posição e tamanho de uma árvore a ser plantada
// ─────────────────────────────────────────────────────────────────────────────
struct TreeData {
    int wx, wz;       // posição mundo (base do tronco)
    int trunkHeight;  // altura do tronco
    int crownRadius;  // raio da copa de folhas
};

// ─────────────────────────────────────────────────────────────────────────────
// TerrainGenerator
// ─────────────────────────────────────────────────────────────────────────────
class TerrainGenerator {
public:
    struct TerrainParams {
        float surfaceLevel           = 40.0f;
        float surfaceHeightVariation = 24.0f;
        float solidBlockDepth        = 8.0f;
        float thresholdShallow       = 0.35f;
        float thresholdMid           = 0.45f;
        float thresholdDeep          = 0.55f;
        float caveThreshold          = 0.50f;
        float maxCaveGenerationDepth = 50.0f;
        int   waterLevel             = 38;   // blocos abaixo desta cota = água
    };

    explicit TerrainGenerator(unsigned int seed);

    // ── Geração principal ─────────────────────────────────────────────────
    void generateChunkBlocks(
        BlockType* blocks,
        int chunkX, int chunkY, int chunkZ,
        int sizeX,  int sizeY,  int sizeZ
    ) const;

    // ── Estruturas pós-geração (árvores) ──────────────────────────────────
    // Retorna árvores cujas bases estão dentro do chunk dado.
    // Deve ser chamado pelo World após gerar blocos de todos os chunks,
    // pois copas podem cruzar bordas de chunks.
    std::vector<TreeData> getTreesForChunk(
        int chunkX, int chunkZ,
        int sizeX,  int sizeZ
    ) const;

    // ── Queries ───────────────────────────────────────────────────────────
    float  getSurfaceHeight(float worldX, float worldZ) const;
    Biome  getBiome        (float worldX, float worldZ) const;
    int    getWaterLevel   () const { return params.waterLevel; }

    void                 setTerrainParams(const TerrainParams& p);
    const TerrainParams& getTerrainParams() const;
    void                 resetToDefaultParams();

    unsigned int getSeed() const { return seed; }

private:
    math::Noise   noise;
    math::Noise   biomeNoise;   // ruído separado para mapa de biomas
    math::Noise   structNoise;  // ruído para posicionamento de estruturas
    TerrainParams params;
    unsigned int  seed;

    BlockType determinBlockType(
        float worldX, float worldY, float worldZ,
        float surfaceHeight, Biome biome
    ) const;

    // Altura de superfície específica por bioma
    float getBiomeSurfaceHeight(float worldX, float worldZ, Biome biome) const;
};

} // namespace fractal_engine::world