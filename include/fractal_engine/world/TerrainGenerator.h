#pragma once
#include <third_party/glm/glm.hpp>
#include "SimpleNoise.h"

namespace fractal_engine::world {

enum BlockType : uint8_t;

class TerrainGenerator {
public:
    struct TerrainParams {
        float surfaceLevel            = 40.0f;
        float surfaceHeightVariation  = 48.0f;
        float surfaceNoiseScale       = 120.0f;

        float caveThreshold           = 0.50f;
        float caveNoiseScale1         = 100.0f;
        float caveNoiseScale2         = 40.0f;

        float solidBlockDepth         = 8.0f;
        float maxCaveGenerationDepth  = 50.0f;

        float thresholdShallow        = 0.35f;
        float thresholdMid            = 0.45f;
        float thresholdDeep           = 0.55f;
    };

    TerrainGenerator(unsigned int seed = 1337);

    void generateChunkBlocks(
        BlockType* blocks,
        int chunkX, int chunkY, int chunkZ,
        int sizeX,  int sizeY,  int sizeZ
    ) const;

    // Público: usado pelo Player para calcular spawn sem depender dos chunks
    float getSurfaceHeight(float worldX, float worldZ) const;

    void                  setTerrainParams(const TerrainParams& params);
    const TerrainParams&  getTerrainParams() const;
    void                  resetToDefaultParams();

private:
    SimpleNoise   noise;
    TerrainParams params;

    BlockType determinBlockType(
        float worldX, float worldY, float worldZ, float surfaceHeight
    ) const;
};

} // namespace fractal_engine::world