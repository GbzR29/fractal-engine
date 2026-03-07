#include "fractal_engine/world/terrain/ChunkMesh.h"
#include <utility>

namespace fractal_engine::world {

// ─────────────────────────────────────────────
// Destrutor
// ─────────────────────────────────────────────
ChunkMesh::~ChunkMesh() {
    freeGPU();
}

// ─────────────────────────────────────────────
// Move
// ─────────────────────────────────────────────
ChunkMesh::ChunkMesh(ChunkMesh&& other) noexcept
    : vertices(std::move(other.vertices))
    , vao(other.vao)
    , vbo(other.vbo)
{
    other.vao = 0;
    other.vbo = 0;
}

ChunkMesh& ChunkMesh::operator=(ChunkMesh&& other) noexcept {
    if (this != &other) {
        freeGPU();
        vertices  = std::move(other.vertices);
        vao       = other.vao;
        vbo       = other.vbo;
        other.vao = 0;
        other.vbo = 0;
    }
    return *this;
}

// ─────────────────────────────────────────────
// GPU
// ─────────────────────────────────────────────
void ChunkMesh::initGPUBuffers() {
    if (vao == 0) {
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
    }
}

void ChunkMesh::setupAttribs() const {
    // attrib 0 — position (x, y, z)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                          STRIDE * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // attrib 1 — UV (u, v)
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,
                          STRIDE * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // attrib 2 — faceType (0=top, 1=bottom, 2=side)
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE,
                          STRIDE * sizeof(float), (void*)(5 * sizeof(float)));
    glEnableVertexAttribArray(2);
}

void ChunkMesh::upload() {
    if (vertices.empty()) return;

    initGPUBuffers();

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 (GLsizeiptr)(vertices.size() * sizeof(float)),
                 vertices.data(),
                 GL_DYNAMIC_DRAW);

    setupAttribs();
    glBindVertexArray(0);
}

void ChunkMesh::draw() const {
    if (vao == 0 || vertices.empty()) return;
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, vertexCount());
    glBindVertexArray(0);
}

void ChunkMesh::freeGPU() {
    if (vao != 0) { glDeleteVertexArrays(1, &vao); vao = 0; }
    if (vbo != 0) { glDeleteBuffers(1, &vbo);       vbo = 0; }
}

} // namespace fractal_engine::world