#pragma once
#include <cstdint>

namespace fractal_engine::world {

// ─────────────────────────────────────────────────────────────────────────────
// BlockType — enum compartilhado por todos os arquivos do mundo
//
// ATENÇÃO: os valores numéricos coincidem com os índices de textura
// usados em Chunk.cpp (topFace/botFace/sideFace). Não reordene sem
// atualizar o switch em generateMesh().
// ─────────────────────────────────────────────────────────────────────────────
enum BlockType : uint8_t {
    BLOCK_AIR   = 0,
    BLOCK_GRASS = 3,   // top=0  bot=1  side=2
    BLOCK_DIRT  = 2,   // top=1  bot=1  side=1
    BLOCK_STONE = 1,   // top=3  bot=3  side=3
    BLOCK_SAND  = 4,
    BLOCK_WATER = 5,
    BLOCK_WOOD  = 6,
    BLOCK_LEAF  = 7,

    BLOCK_COUNT
};

inline bool isTransparent(BlockType t) { return t == BLOCK_AIR || t == BLOCK_WATER; }
inline bool isSolid(BlockType t)       { return t != BLOCK_AIR && t != BLOCK_WATER; }

} // namespace fractal_engine::world