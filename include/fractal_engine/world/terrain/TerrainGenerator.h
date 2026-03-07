#pragma once
#include "fractal_engine/math/Noise.h"
#include "fractal_engine/world/BlockType.h"

namespace fractal_engine::world {

// ─────────────────────────────────────────────────────────────────────────────
// TerrainGenerator — gera blocos proceduralmente via Perlin Noise
//
// Migrado de Perlin3D (hash simples) para math::Noise (Improved Perlin 2002),
// que corrige o bug de índices negativos na tabela de permutação para
// coordenadas de mundo negativas (X ou Z < 0).
//
// A assinatura pública é idêntica à versão anterior — nenhum código
// que chama TerrainGenerator precisa ser alterado.
// ─────────────────────────────────────────────────────────────────────────────
class TerrainGenerator {
public:
    struct TerrainParams {
        // ── Superfície ─────────────────────────────────────────────────────
        float surfaceLevel           = 40.0f;  // altura base do terreno
        float surfaceHeightVariation = 48.0f;  // variação máxima em blocos
        float surfaceNoiseScale      = 120.0f; // passado como 1/scale para fractalNoise

        // ── Crosta sólida (sem cavernas perto da superfície) ───────────────
        float solidBlockDepth        = 8.0f;

        // ── Cavernas (Perlin 3D) ───────────────────────────────────────────
        float caveNoiseScale1        = 100.0f; // cavernas largas
        float caveNoiseScale2        = 40.0f;  // detalhes finos

        // Limiares de densidade para abertura de caverna (noise > threshold → ar)
        // NOTA: math::Noise retorna [0,1] — valores acima de 0.5 são "sólidos"
        // ajustados para dar cavernas razoáveis
        float thresholdShallow       = 0.35f;  // < solidBlockDepth
        float thresholdMid           = 0.45f;  // 15~30 blocos de profundidade
        float thresholdDeep          = 0.55f;  // > 30 blocos

        // Parâmetro legado — mantido para compatibilidade, não usado
        float caveThreshold          = 0.50f;
        float maxCaveGenerationDepth = 50.0f;
    };

    explicit TerrainGenerator(unsigned int seed = 1337);

    // ── Geração ───────────────────────────────────────────────────────────
    // Preenche array blocks[sizeX * sizeY * sizeZ] no layout [x][y][z]
    void generateChunkBlocks(
        BlockType* blocks,
        int chunkX, int chunkY, int chunkZ,
        int sizeX,  int sizeY,  int sizeZ
    ) const;

    // Altura da superfície em coordenadas de mundo (usado pelo Player e HeightMap)
    float getSurfaceHeight(float worldX, float worldZ) const;

    // ── Parâmetros ────────────────────────────────────────────────────────
    void                 setTerrainParams(const TerrainParams& p);
    const TerrainParams& getTerrainParams() const;
    void                 resetToDefaultParams();

private:
    math::Noise   noise;   // substitui Perlin3D — corrige bug de coords negativas
    TerrainParams params;

    BlockType determinBlockType(
        float worldX, float worldY, float worldZ,
        float surfaceHeight
    ) const;
};

} // namespace fractal_engine::world