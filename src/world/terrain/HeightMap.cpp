#include "fractal_engine/world/terrain/HeightMap.h"

namespace fractal_engine::world {

HeightMap::HeightMap(const TerrainGenerator& generator)
    : gen(generator)
{}

float HeightMap::getHeight(int worldX, int worldZ) {
    auto key = std::make_pair(worldX, worldZ);
    auto it  = cache.find(key);

    if (it != cache.end()) {
        hits++;
        return it->second;
    }

    misses++;
    float h = gen.getSurfaceHeight((float)worldX, (float)worldZ);
    cache.emplace(key, h);
    return h;
}

void HeightMap::precompute(int minX, int minZ, int maxX, int maxZ) {
    for (int x = minX; x <= maxX; x++) {
        for (int z = minZ; z <= maxZ; z++) {
            getHeight(x, z);  // preenche o cache
        }
    }
}

void HeightMap::clear() {
    cache.clear();
    hits   = 0;
    misses = 0;
}

} // namespace fractal_engine::world