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
    BLOCK_SNOW        = 9,   // futuro: bioma neve
    BLOCK_CACTUS      = 10,  // futuro: bioma deserto
    BLOCK_COUNT
};

inline bool isTransparent(BlockType t) {
    return t == BLOCK_AIR || t == BLOCK_WATER || t == BLOCK_LEAF;
}
inline bool isSolid(BlockType t) {
    return t != BLOCK_AIR && t != BLOCK_WATER;
}
inline bool isPlaceable(BlockType t) {
    return t != BLOCK_AIR && t != BLOCK_COUNT;
}
inline bool isOpaque(BlockType t) {
    return t != BLOCK_AIR && t != BLOCK_WATER && t != BLOCK_LEAF;
}

// Folha = bloco que usa render separado sem face culling
inline bool isLeafLike(BlockType t) {
    return t == BLOCK_LEAF;
}

inline int getLightEmission(BlockType t) {
    switch (t) {
        default: return 0;
    }
}

} // namespace fractal_engine::world