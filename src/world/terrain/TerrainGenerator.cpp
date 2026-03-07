#include "fractal_engine/world/terrain/TerrainGenerator.h"
#include "fractal_engine/world/terrain/Chunk.h"
#include <cmath>
#include <algorithm>

namespace fractal_engine::world {

TerrainGenerator::TerrainGenerator(unsigned int seed)
    : noise(seed)
{
    resetToDefaultParams();
}

void TerrainGenerator::setTerrainParams(const TerrainParams& newParams) { params = newParams; }
const TerrainGenerator::TerrainParams& TerrainGenerator::getTerrainParams() const { return params; }
void TerrainGenerator::resetToDefaultParams() { params = TerrainParams(); }

// ─────────────────────────────────────────────────────────────────────────────
// getSurfaceHeight
//
// FIX: O antigo Perlin3D::fractalNoise recebia `scale` como divisor de
// frequência interno (freq = 1/scale). O math::Noise::fractalNoise recebe
// `persistence` (amplitude por oitava, ~0.5f).
//
// As coordenadas já chegam pré-multiplicadas (worldX * 0.008f), então
// a frequência de amostragem já está correta — só precisamos passar
// persistence adequado em vez do valor de scale antigo (120.0f).
// ─────────────────────────────────────────────────────────────────────────────
float TerrainGenerator::getSurfaceHeight(float worldX, float worldZ) const
{
    // 0.008f controla o zoom do terreno (menor = colinas mais largas)
    // persistence 0.5f = cada oitava tem metade da amplitude da anterior
    float surfaceHeightNoise = noise.fractalNoise(
        worldX * 0.008f,
        worldZ * 0.008f,
        0.5f,   // persistence — era params.surfaceNoiseScale (120.0f), ERRADO
        5
    );
    return params.surfaceLevel + (surfaceHeightNoise - 0.5f) * params.surfaceHeightVariation;
}

BlockType TerrainGenerator::determinBlockType(
    float worldX, float worldY, float worldZ, float surfaceHeight) const
{
    float depthFromSurface = surfaceHeight - worldY;

    // Camada 1: acima da superfície → ar
    if (depthFromSurface < 0.0f)
        return BLOCK_AIR;

    // Camada 2: crosta sólida (sem cavernas perto da superfície)
    if (depthFromSurface < params.solidBlockDepth) {
        if      (depthFromSurface < 1.0f) return BLOCK_GRASS;
        else if (depthFromSurface < 4.0f) return BLOCK_DIRT;
        else                              return BLOCK_STONE;
    }

    // Camada 3: profundo → pedra com cavernas via Perlin 3D
    //
    // FIX: mesma correção de persistence aplicada ao noise 3D.
    // 0.008f = cavernas largas (baixa frequência)
    // 0.025f = detalhes finos  (alta frequência)
    // persistence 0.5f / 0.4f = decaimento normal por oitava

    float caveLargeNoise = noise.fractalNoise3D(
        worldX * 0.008f,
        worldY * 0.008f,
        worldZ * 0.008f,
        0.5f,   // era params.caveNoiseScale1 (100.0f), ERRADO
        4
    );

    float caveSmallNoise = noise.fractalNoise3D(
        worldX * 0.025f,
        worldY * 0.025f,
        worldZ * 0.025f,
        0.4f,   // era params.caveNoiseScale2 (40.0f), ERRADO
        4
    );

    float caveDensity = caveLargeNoise * 0.7f + caveSmallNoise * 0.3f;

    // Limiar por profundidade: mais fundo = mais cavernas
    float threshold;
    if      (depthFromSurface < 15.0f) threshold = params.thresholdShallow;
    else if (depthFromSurface < 30.0f) threshold = params.thresholdMid;
    else                               threshold = params.thresholdDeep;

    return (caveDensity > threshold) ? BLOCK_STONE : BLOCK_AIR;
}

void TerrainGenerator::generateChunkBlocks(
    BlockType* blocks,
    int chunkX, int chunkY, int chunkZ,
    int sizeX,  int sizeY,  int sizeZ) const
{
    for (int x = 0; x < sizeX; x++) {
    for (int y = 0; y < sizeY; y++) {
    for (int z = 0; z < sizeZ; z++) {
        float worldX = (float)(chunkX + x);
        float worldY = (float)(chunkY + y);
        float worldZ = (float)(chunkZ + z);

        float     surfaceHeight = getSurfaceHeight(worldX, worldZ);
        BlockType blockType     = determinBlockType(worldX, worldY, worldZ, surfaceHeight);

        int idx = x * (sizeY * sizeZ) + y * sizeZ + z;
        blocks[idx] = blockType;
    }}}
}

} // namespace fractal_engine::world