#pragma once
#include <cstdint>

namespace fractal_engine::world {

// ─────────────────────────────────────────────────────────────────────────────
// BlockType — enum sequencial (0, 1, 2, ...)
//
// FIX: os valores anteriores eram não-sequenciais (GRASS=3, DIRT=2, STONE=1),
// causando buracos no array BlockRegistry::defs[] e índices de textura
// incorretos. Agora os valores são puramente sequenciais — a associação
// de texturas é feita inteiramente pelo BlockRegistry, não pelo valor do enum.
//
// PARA ADICIONAR UM NOVO BLOCO:
//   1. Adicione BLOCK_NOME aqui antes de BLOCK_COUNT
//   2. Registre as texturas em BlockRegistry.cpp
//   Pronto.
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

inline bool isTransparent(BlockType t) { return t == BLOCK_AIR || t == BLOCK_WATER; }
inline bool isSolid(BlockType t)       { return t != BLOCK_AIR && t != BLOCK_WATER; }
inline bool isPlaceable(BlockType t)   { return t != BLOCK_AIR && t != BLOCK_COUNT; }

} // namespace fractal_engine::world