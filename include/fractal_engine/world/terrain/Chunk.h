#pragma once
#include <vector>
#include <functional>
#include <third_party/glad/glad.h>
#include <third_party/glm/glm.hpp>
#include <third_party/glm/gtc/matrix_transform.hpp>
#include "fractal_engine/world/BlockType.h"
#include "fractal_engine/graphics/Shader.h"
#include "fractal_engine/graphics/TextureLoader.h"

namespace fractal_engine::world {

using fractal_engine::graphics::Shader;
using fractal_engine::graphics::TextureLoader;

// ─────────────────────────────────────────────────────────────────────────────
// Chunk — unidade de 16×64×16 blocos do mundo
//
// Responsabilidades:
//   - Armazenar o array de blocos (CPU)
//   - Gerar a mesh de faces visíveis (CPU)
//   - Fazer upload e draw da mesh (GPU)
//
// A geração de blocos é delegada ao TerrainGenerator (via generateBlocks()).
// A mesh pode consultar chunks vizinhos via callback worldIsAir para evitar
// faces nas bordas internas entre chunks.
// ─────────────────────────────────────────────────────────────────────────────
class Chunk {
public:
    static constexpr int SIZE_X = 16;
    static constexpr int SIZE_Y = 64;
    static constexpr int SIZE_Z = 16;
    static constexpr int STRIDE = 6;   // floats por vértice: x,y,z,u,v,faceType

    glm::vec3 position;  // canto inferior do chunk em coordenadas de mundo

    Chunk(glm::vec3 pos, Shader& shader);
    ~Chunk();

    // Não copiável (possui VAO/VBO e texturas OpenGL)
    Chunk(const Chunk&)            = delete;
    Chunk& operator=(const Chunk&) = delete;

    // Movível (necessário para std::unordered_map)
    Chunk(Chunk&& other) noexcept;
    Chunk& operator=(Chunk&& other) noexcept;

    // ── Geração ───────────────────────────────────────────────────────────
    // Preenche blocks[] via TerrainGenerator
    void generateBlocks();

    // Gera a mesh de faces visíveis.
    // worldIsAir: callback opcional para consultar blocos fora deste chunk.
    //   signature: bool(int worldX, int worldY, int worldZ)
    void generateMesh(std::function<bool(int,int,int)> worldIsAir = nullptr);

    // Envia a mesh atual para a GPU (chame após generateMesh)
    void uploadMesh();

    // ── Render ────────────────────────────────────────────────────────────
    void Draw(Shader& shader);

    // ── Acesso a blocos (coordenadas LOCAIS) ──────────────────────────────
    bool      isAir     (int x, int y, int z) const;
    BlockType getBlock  (int x, int y, int z) const;
    void      setBlock  (int x, int y, int z, BlockType type);

private:
    BlockType          blocks[SIZE_X][SIZE_Y][SIZE_Z] {};
    std::vector<float> vertices;
    GLuint             vao      = 0;
    GLuint             vbo      = 0;
    GLuint             textures[4] {};  // [0]=top [1]=bottom [2]=side [3]=stone

    void addFace(const float* face, int x, int y, int z, float faceType);
};

} // namespace fractal_engine::world