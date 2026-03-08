#pragma once
#include "fractal_engine/world/BlockType.h"
#include <third_party/glad/glad.h>
#include <string>
#include <array>
#include <vector>

namespace fractal_engine::world {

// ─────────────────────────────────────────────────────────────────────────────
// BlockDef — tudo que define um bloco
// ─────────────────────────────────────────────────────────────────────────────
struct BlockDef {
    // ── Identidade ────────────────────────────────────────────────────────
    std::string name;         // id interno:  "cobblestone"
    std::string displayName;  // nome visível: "Cobblestone"

    // ── Texturas (índices no GL_TEXTURE_2D_ARRAY) ─────────────────────────
    int topLayer  = 0;
    int botLayer  = 0;
    int sideLayer = 0;

    // ── Propriedades ──────────────────────────────────────────────────────
    bool placeable    = true;   // pode ser colocado pelo player
    bool transparent  = false;  // não culls faces vizinhas
    bool solid        = true;   // colide com o player
};

// ─────────────────────────────────────────────────────────────────────────────
// BlockRegistry — registro central de todos os blocos
//
// PARA ADICIONAR UM BLOCO:
//   1. BlockType.h  — adiciona BLOCK_NOME antes de BLOCK_COUNT
//   2. BlockRegistry.cpp — chama registerBlock(BLOCK_NOME, ...) em init()
//   Nenhuma outra alteração necessária.
// ─────────────────────────────────────────────────────────────────────────────
class BlockRegistry {
public:
    static void init();
    static void shutdown();

    // Bind do texture array para draw
    static void bind(int slot = 0);

    // Retorna a definição completa de um bloco
    static const BlockDef& get(BlockType type);

    // ── Iteração ──────────────────────────────────────────────────────────
    // Lista de todos os blocos colocáveis (para hotbar, inventário, etc.)
    // Não contém BLOCK_AIR nem BLOCK_COUNT.
    static const std::vector<BlockType>& getPlaceableBlocks();

    // Número de blocos colocáveis registrados
    static int placeableCount() { return (int)placeableBlocks.size(); }

    static GLuint getTextureArray() { return texArray; }
    static int    layerCount()      { return (int)layers.size(); }

private:
    static std::array<BlockDef, BLOCK_COUNT> defs;
    static std::vector<BlockType>            placeableBlocks;
    static GLuint                            texArray;

    struct LayerEntry { std::string path; };
    static std::vector<LayerEntry> layers;

    // Faces diferentes por face
    static void registerBlock(BlockType          type,
                               const std::string& name,
                               const std::string& displayName,
                               const std::string& topPath,
                               const std::string& botPath,
                               const std::string& sidePath,
                               bool               placeable = true);

    // Mesma textura em todas as faces
    static void registerBlock(BlockType          type,
                               const std::string& name,
                               const std::string& displayName,
                               const std::string& allFacesPath,
                               bool               placeable = true);

    static int  getOrAddLayer(const std::string& path);
    static void uploadTextureArray();
};

} // namespace fractal_engine::world