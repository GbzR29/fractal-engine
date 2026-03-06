#include "fractal_engine/world/Chunk.h"
#include "fractal_engine/world/TerrainGenerator.h"
#include <iostream>
#include <cmath>

namespace fractal_engine::world {

// ─────────────────────────────────────────────────────────────────────────────
// FIX: Faces agora usam coordenadas de CANTO [0, 1] em vez de CENTRO [-0.5, 0.5]
//
// O sistema de raycasting e colisão trata um bloco em (x, y, z) como ocupando
// o cubo [x, x+1] × [y, y+1] × [z, z+1].
// Antes as faces usavam centro em (x, y, z) com extensão ±0.5 — isso causava
// um deslocamento de meio bloco entre visual e lógica.
//
// Cada vértice: x, y, z, u, v, faceType
// faceType: 0=topo, 1=fundo, 2=lateral
// ─────────────────────────────────────────────────────────────────────────────

static const float FACE_RIGHT[] = {   // face +X (x = 1)
    1,0,0,  0,0, 2,
    1,1,0,  0,1, 2,
    1,1,1,  1,1, 2,
    1,1,1,  1,1, 2,
    1,0,1,  1,0, 2,
    1,0,0,  0,0, 2,
};
static const float FACE_LEFT[] = {    // face -X (x = 0)
    0,0,1,  0,0, 2,
    0,1,1,  0,1, 2,
    0,1,0,  1,1, 2,
    0,1,0,  1,1, 2,
    0,0,0,  1,0, 2,
    0,0,1,  0,0, 2,
};
static const float FACE_TOP[] = {     // face +Y (y = 1)
    0,1,1,  0,0, 0,
    1,1,1,  1,0, 0,
    1,1,0,  1,1, 0,
    1,1,0,  1,1, 0,
    0,1,0,  0,1, 0,
    0,1,1,  0,0, 0,
};
static const float FACE_BOTTOM[] = {  // face -Y (y = 0)
    0,0,0,  0,1, 1,
    1,0,0,  1,1, 1,
    1,0,1,  1,0, 1,
    1,0,1,  1,0, 1,
    0,0,1,  0,0, 1,
    0,0,0,  0,1, 1,
};
static const float FACE_FRONT[] = {   // face +Z (z = 1)
    0,0,1,  0,0, 2,
    1,0,1,  1,0, 2,
    1,1,1,  1,1, 2,
    1,1,1,  1,1, 2,
    0,1,1,  0,1, 2,
    0,0,1,  0,0, 2,
};
static const float FACE_BACK[] = {    // face -Z (z = 0)
    1,0,0,  1,0, 2,
    0,0,0,  0,0, 2,
    0,1,0,  0,1, 2,
    0,1,0,  0,1, 2,
    1,1,0,  1,1, 2,
    1,0,0,  1,0, 2,
};

// ─────────────────────────────────────────────
// Construtor
// ─────────────────────────────────────────────
Chunk::Chunk(glm::vec3 pos, Shader& shader) : position(pos) {
    generateBlocks();
    generateMesh();

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenTextures(4, textures);

    uploadMesh();

    TextureLoader(textures[0], "assets/grass_top.png");
    TextureLoader(textures[1], "assets/dirt.png");
    TextureLoader(textures[2], "assets/grass_side.png");
    TextureLoader(textures[3], "assets/stone.png");

    shader.use();
    shader.setInt("texTop",    0);
    shader.setInt("texBottom", 1);
    shader.setInt("texSide",   2);
    shader.setInt("texStone",  3);
}

Chunk::~Chunk() {
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteTextures(4, textures);
}

// ─────────────────────────────────────────────
// Geração de blocos com TerrainGenerator
// ─────────────────────────────────────────────
void Chunk::generateBlocks() {
    static TerrainGenerator terrainGen(1337);
    terrainGen.generateChunkBlocks(
        &blocks[0][0][0],
        (int)position.x,
        (int)position.y,
        (int)position.z,
        SIZE_X, SIZE_Y, SIZE_Z
    );
}

// ─────────────────────────────────────────────
// Acessors
// ─────────────────────────────────────────────
bool Chunk::isAir(int x, int y, int z) const {
    if (x < 0 || x >= SIZE_X || y < 0 || y >= SIZE_Y || z < 0 || z >= SIZE_Z)
        return true;
    return blocks[x][y][z] == BLOCK_AIR;
}

BlockType Chunk::getBlock(int x, int y, int z) const {
    if (x < 0 || x >= SIZE_X || y < 0 || y >= SIZE_Y || z < 0 || z >= SIZE_Z)
        return BLOCK_AIR;
    return blocks[x][y][z];
}

void Chunk::setBlock(int x, int y, int z, BlockType blockType) {
    if (x < 0 || x >= SIZE_X || y < 0 || y >= SIZE_Y || z < 0 || z >= SIZE_Z)
        return;
    blocks[x][y][z] = blockType;
}

// ─────────────────────────────────────────────
// Geração de mesh com suporte a vizinhos
// ─────────────────────────────────────────────
void Chunk::generateMesh(std::function<bool(int,int,int)> worldIsAir) {
    vertices.clear();

    for (int x = 0; x < SIZE_X; x++) {
    for (int y = 0; y < SIZE_Y; y++) {
    for (int z = 0; z < SIZE_Z; z++) {
        if (blocks[x][y][z] == BLOCK_AIR) continue;

        float topFace, botFace, sideFace;

        switch (blocks[x][y][z]) {
            case BLOCK_GRASS:
                topFace = 0; botFace = 1; sideFace = 2; break;
            case BLOCK_STONE:
                topFace = botFace = sideFace = 3; break;
            case BLOCK_DIRT:
            default:
                topFace = botFace = sideFace = 1; break;
        }

        auto neighborIsAir = [&](int lx, int ly, int lz) -> bool {
            if (lx >= 0 && lx < SIZE_X &&
                ly >= 0 && ly < SIZE_Y &&
                lz >= 0 && lz < SIZE_Z)
                return isAir(lx, ly, lz);
            if (worldIsAir) {
                int wx = (int)position.x + lx;
                int wy = (int)position.y + ly;
                int wz = (int)position.z + lz;
                return worldIsAir(wx, wy, wz);
            }
            return true;
        };

        if (neighborIsAir(x+1, y,   z  )) addFace(FACE_RIGHT,  x, y, z, sideFace);
        if (neighborIsAir(x-1, y,   z  )) addFace(FACE_LEFT,   x, y, z, sideFace);
        if (neighborIsAir(x,   y+1, z  )) addFace(FACE_TOP,    x, y, z, topFace);
        if (neighborIsAir(x,   y-1, z  )) addFace(FACE_BOTTOM, x, y, z, botFace);
        if (neighborIsAir(x,   y,   z+1)) addFace(FACE_FRONT,  x, y, z, sideFace);
        if (neighborIsAir(x,   y,   z-1)) addFace(FACE_BACK,   x, y, z, sideFace);
    }}}
}

// ─────────────────────────────────────────────
// Upload para GPU
// ─────────────────────────────────────────────
void Chunk::uploadMesh() {
    if (vao == 0) {
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
    }

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 vertices.size() * sizeof(float),
                 vertices.data(),
                 GL_DYNAMIC_DRAW);

    // layout: position(3), uv(2), faceType(1)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, STRIDE * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, STRIDE * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, STRIDE * sizeof(float), (void*)(5 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}

// ─────────────────────────────────────────────
// addFace
// ─────────────────────────────────────────────
void Chunk::addFace(const float* face, int x, int y, int z, float faceType) {
    for (int i = 0; i < 6 * STRIDE; i += STRIDE) {
        vertices.push_back(face[i + 0] + x);
        vertices.push_back(face[i + 1] + y);
        vertices.push_back(face[i + 2] + z);
        vertices.push_back(face[i + 3]);   // u
        vertices.push_back(face[i + 4]);   // v
        vertices.push_back(faceType);      // sobrescreve faceType da face
    }
}

// ─────────────────────────────────────────────
// Draw
// ─────────────────────────────────────────────
void Chunk::Draw(Shader& shader) {
    if (vertices.empty()) return;

    for (int i = 0; i < 4; i++) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, textures[i]);
    }

    glm::mat4 model = glm::translate(glm::mat4(1.0f), position);
    shader.setMat4("model", model);

    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, (GLsizei)(vertices.size() / STRIDE));
    glBindVertexArray(0);
}

} // namespace fractal_engine::world