#include "fractal_engine/world/terrain/Chunk.h"
#include "fractal_engine/world/terrain/TerrainGenerator.h"
#include "fractal_engine/world/BlockRegistry.h"
#include <cmath>

namespace fractal_engine::world {

// ─────────────────────────────────────────────────────────────────────────────
// Light factors por direção de face
//
// Simula iluminação direcional sem nenhum cálculo de luz em runtime:
// os valores são baked no mesh na hora da geração.
// O shader multiplica por sunIntensity*sunColor para dia/noite.
//
//   TOP    1.0  — face voltada para o sol, mais clara
//   FRONT  0.8  — face +Z
//   BACK   0.8  — face -Z
//   RIGHT  0.6  — face +X
//   LEFT   0.6  — face -X
//   BOTTOM 0.4  — face voltada para baixo, nunca recebe luz direta
// ─────────────────────────────────────────────────────────────────────────────
static constexpr float LF_TOP    = 1.0f;
static constexpr float LF_FRONT  = 0.8f;
static constexpr float LF_BACK   = 0.8f;
static constexpr float LF_RIGHT  = 0.6f;
static constexpr float LF_LEFT   = 0.6f;
static constexpr float LF_BOTTOM = 0.4f;

// Faces: [ x, y, z, u, v, placeholder ]
// O placeholder (último float) é sobrescrito por lightFactor em addFace()
static const float FACE_RIGHT[] = {
    1,0,0, 0,0, 0,
    1,1,0, 0,1, 0,
    1,1,1, 1,1, 0,
    1,1,1, 1,1, 0,
    1,0,1, 1,0, 0,
    1,0,0, 0,0, 0,
};
static const float FACE_LEFT[] = {
    0,0,1, 0,0, 0,
    0,1,1, 0,1, 0,
    0,1,0, 1,1, 0,
    0,1,0, 1,1, 0,
    0,0,0, 1,0, 0,
    0,0,1, 0,0, 0,
};
static const float FACE_TOP[] = {
    0,1,1, 0,0, 0,
    1,1,1, 1,0, 0,
    1,1,0, 1,1, 0,
    1,1,0, 1,1, 0,
    0,1,0, 0,1, 0,
    0,1,1, 0,0, 0,
};
static const float FACE_BOTTOM[] = {
    0,0,0, 0,1, 0,
    1,0,0, 1,1, 0,
    1,0,1, 1,0, 0,
    1,0,1, 1,0, 0,
    0,0,1, 0,0, 0,
    0,0,0, 0,1, 0,
};
static const float FACE_FRONT[] = {
    0,0,1, 0,0, 0,
    1,0,1, 1,0, 0,
    1,1,1, 1,1, 0,
    1,1,1, 1,1, 0,
    0,1,1, 0,1, 0,
    0,0,1, 0,0, 0,
};
static const float FACE_BACK[] = {
    1,0,0, 1,0, 0,
    0,0,0, 0,0, 0,
    0,1,0, 0,1, 0,
    0,1,0, 0,1, 0,
    1,1,0, 1,1, 0,
    1,0,0, 1,0, 0,
};

// ─────────────────────────────────────────────────────────────────────────────
Chunk::Chunk(glm::vec3 pos, Shader& shader) : position(pos) {
    generateBlocks();
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    generateMesh();
    uploadMesh();
}

Chunk::~Chunk() {
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
}

void Chunk::generateBlocks() {
    static TerrainGenerator terrainGen(1337);
    terrainGen.generateChunkBlocks(
        &blocks[0][0][0],
        (int)position.x, (int)position.y, (int)position.z,
        SIZE_X, SIZE_Y, SIZE_Z
    );
}

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

void Chunk::setBlock(int x, int y, int z, BlockType type) {
    if (x < 0 || x >= SIZE_X || y < 0 || y >= SIZE_Y || z < 0 || z >= SIZE_Z)
        return;
    blocks[x][y][z] = type;
}

// ─────────────────────────────────────────────────────────────────────────────
// generateMesh — passa lightFactor diferente por direção de face
// ─────────────────────────────────────────────────────────────────────────────
void Chunk::generateMesh(std::function<bool(int,int,int)> worldIsAir) {
    vertices.clear();

    for (int x = 0; x < SIZE_X; x++)
    for (int y = 0; y < SIZE_Y; y++)
    for (int z = 0; z < SIZE_Z; z++) {
        BlockType type = blocks[x][y][z];
        if (type == BLOCK_AIR) continue;

        const BlockDef& def = BlockRegistry::get(type);

        auto neighborIsAir = [&](int lx, int ly, int lz) -> bool {
            if (lx >= 0 && lx < SIZE_X &&
                ly >= 0 && ly < SIZE_Y &&
                lz >= 0 && lz < SIZE_Z)
                return isAir(lx, ly, lz);
            if (worldIsAir)
                return worldIsAir(
                    (int)position.x + lx,
                    (int)position.y + ly,
                    (int)position.z + lz);
            return true;
        };

        // texLayer e lightFactor passados juntos por face
        if (neighborIsAir(x+1, y,   z  )) addFace(FACE_RIGHT,  x, y, z, (float)def.sideLayer, LF_RIGHT);
        if (neighborIsAir(x-1, y,   z  )) addFace(FACE_LEFT,   x, y, z, (float)def.sideLayer, LF_LEFT);
        if (neighborIsAir(x,   y+1, z  )) addFace(FACE_TOP,    x, y, z, (float)def.topLayer,  LF_TOP);
        if (neighborIsAir(x,   y-1, z  )) addFace(FACE_BOTTOM, x, y, z, (float)def.botLayer,  LF_BOTTOM);
        if (neighborIsAir(x,   y,   z+1)) addFace(FACE_FRONT,  x, y, z, (float)def.sideLayer, LF_FRONT);
        if (neighborIsAir(x,   y,   z-1)) addFace(FACE_BACK,   x, y, z, (float)def.sideLayer, LF_BACK);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// uploadMesh — attrib 3 agora é texLayer, attrib 4 é lightFactor
// ─────────────────────────────────────────────────────────────────────────────
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

    // attrib 0: position (x, y, z)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                          STRIDE * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // attrib 1: uv (u, v)
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,
                          STRIDE * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // attrib 2: texLayer
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE,
                          STRIDE * sizeof(float), (void*)(5 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // attrib 3: lightFactor ← novo
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE,
                          STRIDE * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(3);

    glBindVertexArray(0);
}

void Chunk::Draw(Shader& shader) {
    if (vertices.empty()) return;

    shader.use();
    BlockRegistry::bind(0);
    shader.setInt("texArray", 0);

    glm::mat4 model = glm::translate(glm::mat4(1.0f), position);
    shader.setMat4("model", model);

    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, (GLsizei)(vertices.size() / STRIDE));
    glBindVertexArray(0);
}

// ─────────────────────────────────────────────────────────────────────────────
// addFace — escreve 7 floats por vértice (STRIDE=7)
// ─────────────────────────────────────────────────────────────────────────────
void Chunk::addFace(const float* face, int x, int y, int z,
                    float texLayer, float lightFactor)
{
    // face[] tem 6 vértices × 6 floats (pos + uv + placeholder)
    // Escrevemos 7 floats por vértice: pos + uv + texLayer + lightFactor
    for (int i = 0; i < 6 * 6; i += 6) {
        vertices.push_back(face[i + 0] + x);
        vertices.push_back(face[i + 1] + y);
        vertices.push_back(face[i + 2] + z);
        vertices.push_back(face[i + 3]);    // u
        vertices.push_back(face[i + 4]);    // v
        vertices.push_back(texLayer);       // substitui placeholder
        vertices.push_back(lightFactor);    // novo
    }
}

} // namespace fractal_engine::world