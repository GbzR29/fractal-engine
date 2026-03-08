#include "fractal_engine/world/BlockRegistry.h"
#include <third_party/stb_image/stb_image.h>
#include <iostream>

namespace fractal_engine::world {

std::array<BlockDef, BLOCK_COUNT> BlockRegistry::defs          {};
std::vector<BlockType>             BlockRegistry::placeableBlocks {};
GLuint                             BlockRegistry::texArray      = 0;
std::vector<BlockRegistry::LayerEntry> BlockRegistry::layers    {};

// ─────────────────────────────────────────────────────────────────────────────
// init
//
// ✅ PARA ADICIONAR UM BLOCO NOVO:
//   1. Adicione BLOCK_NOME em BlockType.h antes de BLOCK_COUNT
//   2. Adicione uma linha registerBlock() aqui
//   Pronto — hotbar, inventário e chunk não precisam de alteração.
// ─────────────────────────────────────────────────────────────────────────────
void BlockRegistry::init() {
    stbi_set_flip_vertically_on_load(true);

    registerBlock(BLOCK_GRASS,
        "grass", "Grass",
        "assets/textures/blocks/grass_origin_top.png",
        "assets/textures/blocks/dirt.png",
        "assets/textures/blocks/grass_origin_side.png"
    );

    registerBlock(BLOCK_DIRT,
        "dirt", "Dirt",
        "assets/textures/blocks/dirt.png"
    );

    registerBlock(BLOCK_STONE,
        "stone", "Stone",
        "assets/textures/blocks/stone.png"
    );

    registerBlock(BLOCK_SAND,
        "sand", "Sand",
        "assets/textures/blocks/white_sand.png"
    );

    registerBlock(BLOCK_WATER,
        "water", "Water",
        "assets/textures/blocks/water.png",
        false  // não colocável pelo player por enquanto
    );

    registerBlock(BLOCK_WOOD,
        "wood", "Wood",
        "assets/textures/blocks/acacia_log_top.png",
        "assets/textures/blocks/acacia_log_top.png",
        "assets/textures/blocks/acacia_log.png"
    );

    registerBlock(BLOCK_LEAF,
        "leaf", "Leaves",
        "assets/textures/blocks/leaves_origin.png"
    );

    registerBlock(BLOCK_COBBLESTONE,
        "cobblestone", "Cobblestone",
        "assets/textures/blocks/cobblestone.png"
    );

    uploadTextureArray();

    std::cout << "[BlockRegistry] " << layers.size()
              << " texturas, " << placeableBlocks.size()
              << " blocos colocaveis registrados.\n";
}

void BlockRegistry::shutdown() {
    if (texArray) { glDeleteTextures(1, &texArray); texArray = 0; }
    layers.clear();
    placeableBlocks.clear();
}

void BlockRegistry::bind(int slot) {
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D_ARRAY, texArray);
}

const BlockDef& BlockRegistry::get(BlockType type) {
    return defs[type];
}

const std::vector<BlockType>& BlockRegistry::getPlaceableBlocks() {
    return placeableBlocks;
}

// ─────────────────────────────────────────────────────────────────────────────
// registerBlock — faces diferentes
// ─────────────────────────────────────────────────────────────────────────────
void BlockRegistry::registerBlock(BlockType type,
                                   const std::string& name,
                                   const std::string& displayName,
                                   const std::string& topPath,
                                   const std::string& botPath,
                                   const std::string& sidePath,
                                   bool placeable)
{
    defs[type].name        = name;
    defs[type].displayName = displayName;
    defs[type].topLayer    = getOrAddLayer(topPath);
    defs[type].botLayer    = getOrAddLayer(botPath);
    defs[type].sideLayer   = getOrAddLayer(sidePath);
    defs[type].placeable   = placeable;
    defs[type].transparent = isTransparent(type);
    defs[type].solid       = isSolid(type);

    if (placeable)
        placeableBlocks.push_back(type);
}

// ─────────────────────────────────────────────────────────────────────────────
// registerBlock — todas as faces iguais
// ─────────────────────────────────────────────────────────────────────────────
void BlockRegistry::registerBlock(BlockType type,
                                   const std::string& name,
                                   const std::string& displayName,
                                   const std::string& allFacesPath,
                                   bool placeable)
{
    registerBlock(type, name, displayName,
                  allFacesPath, allFacesPath, allFacesPath,
                  placeable);
}

// ─────────────────────────────────────────────────────────────────────────────
// getOrAddLayer
// ─────────────────────────────────────────────────────────────────────────────
int BlockRegistry::getOrAddLayer(const std::string& path) {
    for (int i = 0; i < (int)layers.size(); i++)
        if (layers[i].path == path) return i;
    layers.push_back({ path });
    return (int)layers.size() - 1;
}

// ─────────────────────────────────────────────────────────────────────────────
// uploadTextureArray
// ─────────────────────────────────────────────────────────────────────────────
void BlockRegistry::uploadTextureArray() {
    if (layers.empty()) return;

    int texW = 16, texH = 16, channels = 0;
    {
        unsigned char* data = stbi_load(layers[0].path.c_str(),
                                        &texW, &texH, &channels, 4);
        if (!data)
            std::cerr << "[BlockRegistry] ERRO ao carregar: " << layers[0].path << "\n";
        else
            stbi_image_free(data);
    }

    const int count = (int)layers.size();

    glGenTextures(1, &texArray);
    glBindTexture(GL_TEXTURE_2D_ARRAY, texArray);
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA8,
                 texW, texH, count, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    for (int i = 0; i < count; i++) {
        int w, h, ch;
        unsigned char* data = stbi_load(layers[i].path.c_str(), &w, &h, &ch, 4);

        if (!data) {
            std::cerr << "[BlockRegistry] ERRO camada " << i
                      << ": " << layers[i].path << "\n";
            // textura magenta de erro
            std::vector<unsigned char> err(texW * texH * 4);
            for (int p = 0; p < texW * texH; p++) {
                err[p*4+0] = 255; err[p*4+1] = 0;
                err[p*4+2] = 255; err[p*4+3] = 255;
            }
            glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0,
                            0, 0, i, texW, texH, 1,
                            GL_RGBA, GL_UNSIGNED_BYTE, err.data());
            continue;
        }

        glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0,
                        0, 0, i, w, h, 1,
                        GL_RGBA, GL_UNSIGNED_BYTE, data);
        stbi_image_free(data);
    }

    glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S,     GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T,     GL_REPEAT);
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
}

} // namespace fractal_engine::world