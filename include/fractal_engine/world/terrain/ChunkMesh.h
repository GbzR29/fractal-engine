#pragma once
#include <vector>
#include <glad/glad.h>

namespace fractal_engine::world {

// ─────────────────────────────────────────────────────────────────────────────
// ChunkMesh — contêiner de dados de mesh de um chunk
//
// Separa a responsabilidade de armazenar/fazer upload de vértices
// da lógica de geração do Chunk, facilitando futuramente:
//   - Geração de mesh em thread separada (sem contexto OpenGL)
//   - Múltiplos passes (sólido, transparente, etc.)
//   - Level-of-detail (LOD)
//
// Layout de vértice: [ x, y, z, u, v, faceType ]  → STRIDE = 6 floats
// ─────────────────────────────────────────────────────────────────────────────
class ChunkMesh {
public:
    static constexpr int STRIDE = 6;  // floats por vértice

    ChunkMesh() = default;
    ~ChunkMesh();

    // Não copiável (possui recursos OpenGL)
    ChunkMesh(const ChunkMesh&)            = delete;
    ChunkMesh& operator=(const ChunkMesh&) = delete;

    // Movível
    ChunkMesh(ChunkMesh&& other) noexcept;
    ChunkMesh& operator=(ChunkMesh&& other) noexcept;

    // ── Dados de vértice (CPU) ─────────────────────────────────────────────
    std::vector<float> vertices;

    void clear() { vertices.clear(); }
    bool empty() const { return vertices.empty(); }
    int  vertexCount() const { return (int)(vertices.size() / STRIDE); }

    // ── GPU ───────────────────────────────────────────────────────────────
    // Cria VAO/VBO se necessário e faz upload dos vértices para a GPU
    void upload();

    // Desenha os triângulos (bind VAO + glDrawArrays)
    void draw() const;

    // Libera recursos OpenGL
    void freeGPU();

    GLuint vao = 0;
    GLuint vbo = 0;

private:
    void initGPUBuffers();
    void setupAttribs() const;
};

} // namespace fractal_engine::world