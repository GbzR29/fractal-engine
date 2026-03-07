#pragma once
#include "fractal_engine/world/terrain/TerrainGenerator.h"
#include <unordered_map>
#include <utility>

namespace fractal_engine::world {

// ─────────────────────────────────────────────────────────────────────────────
// HeightMap — cache de alturas de superfície por coluna (worldX, worldZ)
//
// Evita recalcular getSurfaceHeight() repetidamente durante geração de chunks,
// remesh de bordas e cálculo de spawn do player.
// ─────────────────────────────────────────────────────────────────────────────
class HeightMap {
public:
    explicit HeightMap(const TerrainGenerator& gen);

    // Retorna altura cacheada (calcula e armazena se não existir)
    float  getHeight(int worldX, int worldZ);

    // Pré-aquece o cache para uma região inteira (útil antes de generateWorld)
    void   precompute(int minX, int minZ, int maxX, int maxZ);

    void   clear();

    size_t cacheSize()   const { return cache.size(); }
    size_t cacheHits()   const { return hits;   }
    size_t cacheMisses() const { return misses; }

private:
    const TerrainGenerator& gen;

    struct PairHash {
        size_t operator()(const std::pair<int,int>& p) const {
            size_t h1 = std::hash<int>{}(p.first);
            size_t h2 = std::hash<int>{}(p.second);
            return h1 ^ (h2 * 2654435761ULL + 0x9e3779b9 + (h1 << 6) + (h1 >> 2));
        }
    };

    std::unordered_map<std::pair<int,int>, float, PairHash> cache;
    mutable size_t hits   = 0;
    mutable size_t misses = 0;
};

} // namespace fractal_engine::world