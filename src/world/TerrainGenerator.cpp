#include "fractal_engine/world/TerrainGenerator.h"
#include "fractal_engine/world/Chunk.h"
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

float TerrainGenerator::getSurfaceHeight(float worldX, float worldZ) const
{
    float surfaceHeightNoise = noise.fractalNoise(
        worldX * 0.008f,
        worldZ * 0.008f,
        params.surfaceNoiseScale,
        5
    );
    return params.surfaceLevel + (surfaceHeightNoise - 0.5f) * params.surfaceHeightVariation;
}

BlockType TerrainGenerator::determinBlockType(
    float worldX, float worldY, float worldZ, float surfaceHeight) const
{
    float depthFromSurface = surfaceHeight - worldY;

    // ─────────────────────────────────────────────────────────────────────
    // Camada 1: Acima da superfície → ar
    // ─────────────────────────────────────────────────────────────────────
    if (depthFromSurface < 0.0f) {
        return BLOCK_AIR;
    }

    // ─────────────────────────────────────────────────────────────────────
    // Camada 2: Crosta sólida perto da superfície (sem cavernas aqui)
    //           solidBlockDepth define quantos blocos são incondicionalmente
    //           sólidos abaixo da superfície (ex: 4.0 = 4 blocos)
    // ─────────────────────────────────────────────────────────────────────
    if (depthFromSurface < params.solidBlockDepth) {
        if      (depthFromSurface < 1.0f) return BLOCK_GRASS;
        else if (depthFromSurface < 4.0f) return BLOCK_DIRT;
        else                              return BLOCK_STONE;
    }

    // ─────────────────────────────────────────────────────────────────────
    // Camada 3: Profundo → pedra com cavernas via Perlin 3D
    //
    // FIX BUG 2: A condição original usava:
    //   worldY < surfaceLevel + maxCaveGenerationDepth
    // Se maxCaveGenerationDepth era negativo, isso excluía quase todos os
    // blocos profundos, fazendo o terreno ficar OCO (returnava BLOCK_AIR
    // no fallback abaixo). Agora usamos depthFromSurface diretamente,
    // que é sempre positivo e não depende do sinal de nenhum parâmetro.
    // ─────────────────────────────────────────────────────────────────────

    // Ruído de baixa frequência → cavernas grandes
    float caveLargeNoise = noise.fractalNoise3D(
        worldX * 0.008f,
        worldY * 0.008f,
        worldZ * 0.008f,
        params.caveNoiseScale1,
        4
    );

    // Ruído de alta frequência → cavernas pequenas e detalhes
    float caveSmallNoise = noise.fractalNoise3D(
        worldX * 0.025f,
        worldY * 0.025f,
        worldZ * 0.025f,
        params.caveNoiseScale2,
        4
    );

    float caveDensity = caveLargeNoise * 0.7f + caveSmallNoise * 0.3f;

    // Limiar ajustado pela profundidade:
    // Mais perto da superfície → menos cavernas (limiar mais alto = mais pedra)
    // Mais fundo              → mais cavernas  (limiar mais baixo = mais ar)
    float threshold;
    if      (depthFromSurface < 15.0f) threshold = params.thresholdShallow;
    else if (depthFromSurface < 30.0f) threshold = params.thresholdMid;
    else                               threshold = params.thresholdDeep;

    // FIX: retorna STONE por padrão (sólido), abre caverna só se abaixo do limiar
    // O código antigo retornava BLOCK_AIR no fallback → terreno completamente oco!
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

                float surfaceHeight = getSurfaceHeight(worldX, worldZ);
                BlockType blockType = determinBlockType(worldX, worldY, worldZ, surfaceHeight);

                // ─────────────────────────────────────────────────────────
                // FIX BUG 1: índice do array 3D estava ERRADO.
                //
                // O Chunk armazena como blocks[SIZE_X][SIZE_Y][SIZE_Z],
                // que em memória é row-major C++:
                //   &blocks[x][y][z] = base + (x*SIZE_Y*SIZE_Z + y*SIZE_Z + z)
                //
                // O código antigo usava:  x + y*sizeX + z*sizeX*sizeY
                // Isso é a ordem [z][y][x], não [x][y][z]!
                // → X e Z ficavam embaralhados → blocos em posições erradas
                //   → getTerrainHeightAt lia blocos incorretos → spawn errado
                // ─────────────────────────────────────────────────────────
                int idx = x * (sizeY * sizeZ) + y * sizeZ + z;
                blocks[idx] = blockType;
            }
        }
    }
}

} // namespace fractal_engine::world