#pragma once
#include <cstdint>

namespace fractal_engine::world {

// ─────────────────────────────────────────────────────────────────────────────
// BlockType — enum sequencial
// PARA ADICIONAR UM NOVO BLOCO:
//   1. Adicione BLOCK_NOME aqui antes de BLOCK_COUNT
//   2. Registre as texturas em BlockRegistry.cpp
//   3. Se emite luz, adicione caso em getLightEmission()
// ─────────────────────────────────────────────────────────────────────────────
enum BlockType : uint8_t {
    BLOCK_AIR         = 0,
    BLOCK_GRASS       = 1,
    BLOCK_DIRT        = 2,
    BLOCK_STONE       = 3,
    BLOCK_SAND        = 4,
    BLOCK_WATER       = 5,
    BLOCK_WOOD        = 6,
    BLOCK_LEAF        = 7,
    BLOCK_COBBLESTONE = 8,
    BLOCK_COUNT   // sempre o último
};

inline bool isTransparent(BlockType t) { return t == BLOCK_AIR || t == BLOCK_WATER || t == BLOCK_LEAF; }
inline bool isSolid(BlockType t)       { return t != BLOCK_AIR && t != BLOCK_WATER; }
inline bool isPlaceable(BlockType t)   { return t != BLOCK_AIR && t != BLOCK_COUNT; }

// Opaco = bloco que bloqueia completamente a luz (nem skylight passa)
inline bool isOpaque(BlockType t) {
    return t != BLOCK_AIR
        && t != BLOCK_WATER
        && t != BLOCK_LEAF;
}

// Emissão de luz do bloco [0-15]
// 0 = não emite luz, 15 = luz máxima (tocha)
inline int getLightEmission(BlockType t) {
    switch (t) {
        // Adicione tochas e outros blocos emissivos aqui no futuro:
        // case BLOCK_TORCH: return 14;
        // case BLOCK_GLOWSTONE: return 15;
        default: return 0;
    }
}

} // namespace fractal_engine::world