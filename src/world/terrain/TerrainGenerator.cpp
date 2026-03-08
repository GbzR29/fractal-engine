#include "fractal_engine/world/terrain/TerrainGenerator.h"
#include "fractal_engine/world/terrain/Chunk.h"
#include <cmath>
#include <algorithm>

namespace fractal_engine::world {

// ─────────────────────────────────────────────────────────────────────────────
// Construtor
// ─────────────────────────────────────────────────────────────────────────────
TerrainGenerator::TerrainGenerator(unsigned int seed)
    : noise      (seed)
    , biomeNoise (seed + 1000)   // offset para ruídos independentes
    , structNoise(seed + 2000)
    , seed(seed)
{
    resetToDefaultParams();
}

void TerrainGenerator::setTerrainParams(const TerrainParams& p) { params = p; }
const TerrainGenerator::TerrainParams& TerrainGenerator::getTerrainParams() const { return params; }
void TerrainGenerator::resetToDefaultParams() { params = TerrainParams(); }

// ─────────────────────────────────────────────────────────────────────────────
// getBiome
//
// Usa dois canais de ruído de baixa frequência para criar um mapa de biomas
// suave. Temperatura e umidade determinam o bioma, como no Minecraft.
//
//   temperatura alta  + umidade baixa  → Deserto
//   temperatura baixa + umidade alta   → Floresta
//   altitude extrema                   → Montanha
//   terreno baixo (< waterLevel+2)     → Oceano
// ─────────────────────────────────────────────────────────────────────────────
Biome TerrainGenerator::getBiome(float worldX, float worldZ) const {
    // Frequência muito baixa = biomas grandes (escala continental)
    float temperature = biomeNoise.fractalNoise(
        worldX * 0.002f, worldZ * 0.002f, 0.5f, 3);

    float humidity = biomeNoise.fractalNoise(
        worldX * 0.002f + 500.0f, worldZ * 0.002f + 500.0f, 0.5f, 3);

    // Altitude base (sem modificadores de bioma)
    float baseHeight = noise.fractalNoise(
        worldX * 0.008f, worldZ * 0.008f, 0.5f, 5);
    float height = params.surfaceLevel + (baseHeight - 0.5f) * params.surfaceHeightVariation;

    // Oceano: regiões muito baixas
    if (height < params.waterLevel + 2)
        return Biome::Ocean;

    // Montanha: ruído de altitude alto
    float mountainNoise = biomeNoise.fractalNoise(
        worldX * 0.003f + 300.0f, worldZ * 0.003f + 300.0f, 0.5f, 4);
    if (mountainNoise > 0.72f)
        return Biome::Mountain;

    // Temperatura e umidade → bioma
    if (temperature > 0.6f && humidity < 0.4f)
        return Biome::Desert;

    if (humidity > 0.55f && temperature > 0.35f)
        return Biome::Forest;

    return Biome::Plains;
}

// ─────────────────────────────────────────────────────────────────────────────
// getBiomeSurfaceHeight
//
// Cada bioma modula a altura de forma diferente:
//   - Montanha: amplitude muito maior, frequência maior
//   - Deserto:  terreno suave e plano
//   - Floresta: colinas suaves
//   - Oceano:   sempre abaixo do nível do mar
// ─────────────────────────────────────────────────────────────────────────────
float TerrainGenerator::getBiomeSurfaceHeight(float worldX, float worldZ, Biome biome) const {
    float base = noise.fractalNoise(worldX * 0.008f, worldZ * 0.008f, 0.5f, 5);

    switch (biome) {
        case Biome::Mountain: {
            // Picos altos: usa oitava extra de alta frequência para detalhes rochosos
            float detail = noise.fractalNoise(worldX * 0.02f, worldZ * 0.02f, 0.4f, 4);
            float height = params.surfaceLevel + (base - 0.5f) * 60.0f + (detail - 0.5f) * 20.0f;
            return std::min(height, (float)(Chunk::SIZE_Y - 4));
        }
        case Biome::Desert:
            // Terreno suave, pouca variação
            return params.surfaceLevel + (base - 0.5f) * 12.0f;

        case Biome::Forest:
            // Colinas suaves um pouco mais altas que plains
            return params.surfaceLevel + (base - 0.5f) * 20.0f;

        case Biome::Ocean: {
            // Fundo do oceano: sempre abaixo do nível da água
            float ocean = noise.fractalNoise(worldX * 0.012f, worldZ * 0.012f, 0.5f, 3);
            return (params.waterLevel - 3) + (ocean - 0.5f) * 6.0f;
        }
        case Biome::Plains:
        default:
            return params.surfaceLevel + (base - 0.5f) * params.surfaceHeightVariation;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// getSurfaceHeight
// ─────────────────────────────────────────────────────────────────────────────
float TerrainGenerator::getSurfaceHeight(float worldX, float worldZ) const {
    Biome biome = getBiome(worldX, worldZ);
    return getBiomeSurfaceHeight(worldX, worldZ, biome);
}

// ─────────────────────────────────────────────────────────────────────────────
// determinBlockType
// ─────────────────────────────────────────────────────────────────────────────
BlockType TerrainGenerator::determinBlockType(
    float worldX, float worldY, float worldZ,
    float surfaceHeight, Biome biome) const
{
    float depthFromSurface = surfaceHeight - worldY;

    // ── Acima da superfície ───────────────────────────────────────────────
    if (depthFromSurface < 0.0f) {
        // Preenche com água abaixo do waterLevel
        if ((int)worldY <= params.waterLevel)
            return BLOCK_WATER;
        return BLOCK_AIR;
    }

    // ── Crosta sólida (sem cavernas próximo à superfície) ─────────────────
    if (depthFromSurface < params.solidBlockDepth) {
        switch (biome) {
            case Biome::Desert:
                // Areia nos primeiros 4 blocos, depois pedra
                if (depthFromSurface < 4.0f) return BLOCK_SAND;
                return BLOCK_STONE;

            case Biome::Mountain:
                // Pedra exposta no topo da montanha
                if (surfaceHeight > params.surfaceLevel + 18.0f) {
                    return BLOCK_STONE;
                }
                if (depthFromSurface < 1.0f) return BLOCK_GRASS;
                if (depthFromSurface < 3.0f) return BLOCK_DIRT;
                return BLOCK_STONE;

            case Biome::Ocean:
                // Fundo do oceano: areia ou pedra
                if (depthFromSurface < 2.0f) return BLOCK_SAND;
                return BLOCK_STONE;

            default: // Plains, Forest
                if (depthFromSurface < 1.0f) return BLOCK_GRASS;
                if (depthFromSurface < 4.0f) return BLOCK_DIRT;
                return BLOCK_STONE;
        }
    }

    // ── Profundo: pedra com cavernas ──────────────────────────────────────
    float caveLarge = noise.fractalNoise3D(
        worldX * 0.008f, worldY * 0.008f, worldZ * 0.008f, 0.5f, 4);
    float caveSmall = noise.fractalNoise3D(
        worldX * 0.025f, worldY * 0.025f, worldZ * 0.025f, 0.4f, 4);
    float caveDensity = caveLarge * 0.7f + caveSmall * 0.3f;

    float threshold;
    if      (depthFromSurface < 15.0f) threshold = params.thresholdShallow;
    else if (depthFromSurface < 30.0f) threshold = params.thresholdMid;
    else                               threshold = params.thresholdDeep;

    return (caveDensity > threshold) ? BLOCK_STONE : BLOCK_AIR;
}

// ─────────────────────────────────────────────────────────────────────────────
// generateChunkBlocks
// ─────────────────────────────────────────────────────────────────────────────
void TerrainGenerator::generateChunkBlocks(
    BlockType* blocks,
    int chunkX, int chunkY, int chunkZ,
    int sizeX,  int sizeY,  int sizeZ) const
{
    for (int x = 0; x < sizeX; x++)
    for (int y = 0; y < sizeY; y++)
    for (int z = 0; z < sizeZ; z++) {
        float wx = (float)(chunkX + x);
        float wy = (float)(chunkY + y);
        float wz = (float)(chunkZ + z);

        Biome     biome         = getBiome(wx, wz);
        float     surfaceHeight = getBiomeSurfaceHeight(wx, wz, biome);
        BlockType blockType     = determinBlockType(wx, wy, wz, surfaceHeight, biome);

        int idx = x * (sizeY * sizeZ) + y * sizeZ + z;
        blocks[idx] = blockType;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// getTreesForChunk
//
// Para cada posição de grade dentro do chunk, usa structNoise para decidir
// se uma árvore deve ser plantada ali. Só floresta e plains geram árvores
// (floresta com maior densidade).
//
// A árvore é definida apenas por posição + dimensões — quem planta os blocos
// é o World, após ter todos os chunks gerados.
// ─────────────────────────────────────────────────────────────────────────────
std::vector<TreeData> TerrainGenerator::getTreesForChunk(
    int chunkX, int chunkZ,
    int sizeX,  int sizeZ) const
{
    std::vector<TreeData> trees;

    // Grade de posições candidatas (a cada ~4 blocos)
    constexpr int GRID = 4;

    for (int lx = 0; lx < sizeX; lx += GRID)
    for (int lz = 0; lz < sizeZ; lz += GRID) {
        int wx = chunkX + lx;
        int wz = chunkZ + lz;

        Biome biome = getBiome((float)wx, (float)wz);
        if (biome != Biome::Forest && biome != Biome::Plains)
            continue;

        // Probabilidade de árvore nesta célula
        float treeDensity = (biome == Biome::Forest) ? 0.55f : 0.18f;
        float treeNoise   = structNoise.fractalNoise(
            wx * 0.25f, wz * 0.25f, 0.5f, 1);

        if (treeNoise < treeDensity) continue;

        // Pequeno deslocamento dentro da célula para evitar grid perfeito
        float offsetX = structNoise.fractalNoise(wx * 0.7f, wz * 0.3f, 0.5f, 1);
        float offsetZ = structNoise.fractalNoise(wx * 0.3f, wz * 0.7f, 0.5f, 1);
        int   fx      = wx + (int)(offsetX * (GRID - 1));
        int   fz      = wz + (int)(offsetZ * (GRID - 1));

        // Altura do tronco: 4-6 blocos
        float heightNoise = structNoise.fractalNoise(fx * 0.5f, fz * 0.5f, 0.5f, 1);
        int trunkH = 4 + (int)(heightNoise * 3.0f); // 4, 5 ou 6

        // Raio da copa: 2-3 blocos
        float crownNoise = structNoise.fractalNoise(fx * 0.4f + 100.0f, fz * 0.4f, 0.5f, 1);
        int crownR = 2 + (crownNoise > 0.6f ? 1 : 0);

        trees.push_back({ fx, fz, trunkH, crownR });
    }

    return trees;
}

} // namespace fractal_engine::world