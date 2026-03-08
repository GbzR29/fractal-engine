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

class Chunk {
public:
    static constexpr int SIZE_X = 16;
    static constexpr int SIZE_Y = 64;
    static constexpr int SIZE_Z = 16;

    // ── STRIDE atualizado ─────────────────────────────────────────────────
    // Layout: [ x, y, z, u, v, texLayer, lightFactor ]
    //   lightFactor: brilho base da face [0,1] — topo=1.0, fundo=0.4, lados=0.6~0.8
    //   Multiplicado no shader por sunIntensity*sunColor para iluminação dinâmica
    static constexpr int STRIDE = 7;

    glm::vec3 position;

    Chunk(glm::vec3 pos, Shader& shader);
    ~Chunk();

    Chunk(const Chunk&)            = delete;
    Chunk& operator=(const Chunk&) = delete;
    Chunk(Chunk&&)                 = default;
    Chunk& operator=(Chunk&&)      = default;

    void generateBlocks();
    void generateMesh(std::function<bool(int,int,int)> worldIsAir = nullptr);
    void uploadMesh();
    void Draw(Shader& shader);

    bool      isAir     (int x, int y, int z) const;
    BlockType getBlock  (int x, int y, int z) const;
    void      setBlock  (int x, int y, int z, BlockType type);

private:
    BlockType          blocks[SIZE_X][SIZE_Y][SIZE_Z] {};
    std::vector<float> vertices;
    GLuint             vao = 0;
    GLuint             vbo = 0;

    // texLayer e lightFactor são passados como parâmetro separado agora
    void addFace(const float* face, int x, int y, int z,
                 float texLayer, float lightFactor);
};

} // namespace fractal_engine::world