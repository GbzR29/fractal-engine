#pragma once
#include <vector>
#include <functional>
#include <cstdint>
#include <third_party/glad/glad.h>
#include <third_party/glm/glm.hpp>
#include <third_party/glm/gtc/matrix_transform.hpp>
#include "fractal_engine/world/BlockType.h"
#include "fractal_engine/graphics/Shader.h"
#include "fractal_engine/graphics/TextureLoader.h"
#include "fractal_engine/world/terrain/TerrainGenerator.h"

namespace fractal_engine::world {

using fractal_engine::graphics::Shader;
using fractal_engine::graphics::TextureLoader;

// ─────────────────────────────────────────────────────────────────────────────
// Chunk — armazena blocos + mapa de luz, gera mesh
//
// SISTEMA DE LUZ:
//   lightMap[x][y][z] = uint8_t empacotado:
//     bits 7-4: skylight  [0-15]  luz do céu
//     bits 3-0: blocklight[0-15]  luz artificial (tochas etc.)
//
//   getLightValue() retorna max(skylight, blocklight) × sunIntensity no shader
//
// STRIDE = 8: [x, y, z, u, v, texLayer, lightFactor, lightValue]
//   lightFactor : float baked por face (top=1.0, sides=0.6~0.8, bottom=0.4)
//   lightValue  : float [0,1] do lightMap, interpolado no shader
// ─────────────────────────────────────────────────────────────────────────────
class Chunk {
public:
    static constexpr int SIZE_X = 16;
    static constexpr int SIZE_Y = 64;
    static constexpr int SIZE_Z = 16;

    // Layout: [x, y, z, u, v, texLayer, lightFactor, lightValue]
    static constexpr int STRIDE = 8;

    // Constantes de luz
    static constexpr int MAX_LIGHT     = 15;
    static constexpr int SKY_LIGHT_MAX = 15;

    glm::vec3 position;

    Chunk(glm::vec3 pos, Shader& shader);
    ~Chunk();

    Chunk(const Chunk&)            = delete;
    Chunk& operator=(const Chunk&) = delete;
    Chunk(Chunk&&)                 = default;
    Chunk& operator=(Chunk&&)      = default;

    // ── Blocos ────────────────────────────────────────────────────────────
    void generateBlocks();
    void generateBlocks(const TerrainGenerator& gen);
    bool      isAir     (int x, int y, int z) const;
    BlockType getBlock  (int x, int y, int z) const;
    void      setBlock  (int x, int y, int z, BlockType type);

    // ── Luz ───────────────────────────────────────────────────────────────
    // Retorna skylight (bits 7-4) do voxel
    int  getSkyLight  (int x, int y, int z) const;
    // Retorna blocklight (bits 3-0) do voxel
    int  getBlockLight(int x, int y, int z) const;
    // Define skylight
    void setSkyLight  (int x, int y, int z, int val);
    // Define blocklight
    void setBlockLight(int x, int y, int z, int val);
    // Retorna max(skylight, blocklight) como float [0,1]
    float getLightValue(int x, int y, int z) const;

    // Inicializa skylight (propaga de cima para baixo dentro do chunk)
    // worldGetSkyLight: getter de luz de fora do chunk (para bordas)
    void initSkyLight(std::function<int(int,int,int)> worldGetSkyLight = nullptr);

    // Propaga blocklight por BFS dentro do chunk
    // worldGetBlock: getter de bloco de fora do chunk (para bordas)
    // worldGetBlockLight: getter de luz de fora do chunk
    // lightSources: lista de posições MUNDO com emissão a propagar
    // worldGetBlock:     bloco de posição mundo (para bordas)
    // worldGetSkyLight:  skylight de posição mundo (para injetar luz lateral dos vizinhos)
    void propagateLight(
        std::function<BlockType(int,int,int)> worldGetBlock    = nullptr,
        std::function<int(int,int,int)>       worldGetSkyLight = nullptr
    );

    // Zera todo o lightMap
    void clearLight();

    // ── Mesh ──────────────────────────────────────────────────────────────
    void generateMesh(std::function<bool(int,int,int)>  worldIsAir    = nullptr,
                      std::function<float(int,int,int)> worldGetLight = nullptr);
    void uploadMesh();

    // Passagem 1: só blocos opacos
    void DrawOpaque     (Shader& shader);
    // Passagem 2: folhas e água (sem culling — controlado pelo World)
    void DrawTransparent(Shader& shader);
    // Compat: chama ambas
    void Draw(Shader& shader);

    // Público — usado por funções livres (getLightForFace) e pelo World
    bool inBounds(int x, int y, int z) const {
        return x >= 0 && x < SIZE_X
            && y >= 0 && y < SIZE_Y
            && z >= 0 && z < SIZE_Z;
    }

private:
    BlockType          blocks  [SIZE_X][SIZE_Y][SIZE_Z] {};
    uint8_t            lightMap[SIZE_X][SIZE_Y][SIZE_Z] {};

    std::vector<float> vertices;            // blocos opacos
    std::vector<float> verticesTransparent; // folhas + água
    GLuint             vao = 0, vbo = 0;
    GLuint             vaoT = 0, vboT = 0; // transparentes

    void addFace(const float* face, int x, int y, int z,
                 float texLayer, float lightFactor, float lightValue);

    // Smooth lighting — luz diferente por vértice
    void addFaceSmooth(const float* face, int x, int y, int z,
                       float texLayer, float lightFactor,
                       float l0, float l1, float l2,
                       float l3, float l4, float l5);
};

} // namespace fractal_engine::world