#include "fractal_engine/world/terrain/Chunk.h"
#include "fractal_engine/world/terrain/TerrainGenerator.h"
#include "fractal_engine/world/BlockRegistry.h"
#include <cmath>
#include <queue>
#include <tuple>

namespace fractal_engine::world {

// ─────────────────────────────────────────────────────────────────────────────
// Light factors por direção de face (baked no mesh, multiplicado pela luz)
// ─────────────────────────────────────────────────────────────────────────────
static constexpr float LF_TOP    = 1.0f;
static constexpr float LF_FRONT  = 0.8f;
static constexpr float LF_BACK   = 0.8f;
static constexpr float LF_RIGHT  = 0.6f;
static constexpr float LF_LEFT   = 0.6f;
static constexpr float LF_BOTTOM = 0.4f;

// Faces: [x, y, z, u, v, placeholder_texLayer]
static const float FACE_RIGHT[] = {
    1,0,0, 0,0, 0,  1,1,0, 0,1, 0,  1,1,1, 1,1, 0,
    1,1,1, 1,1, 0,  1,0,1, 1,0, 0,  1,0,0, 0,0, 0,
};
static const float FACE_LEFT[] = {
    0,0,1, 0,0, 0,  0,1,1, 0,1, 0,  0,1,0, 1,1, 0,
    0,1,0, 1,1, 0,  0,0,0, 1,0, 0,  0,0,1, 0,0, 0,
};
static const float FACE_TOP[] = {
    0,1,1, 0,0, 0,  1,1,1, 1,0, 0,  1,1,0, 1,1, 0,
    1,1,0, 1,1, 0,  0,1,0, 0,1, 0,  0,1,1, 0,0, 0,
};
static const float FACE_BOTTOM[] = {
    0,0,0, 0,1, 0,  1,0,0, 1,1, 0,  1,0,1, 1,0, 0,
    1,0,1, 1,0, 0,  0,0,1, 0,0, 0,  0,0,0, 0,1, 0,
};
static const float FACE_FRONT[] = {
    0,0,1, 0,0, 0,  1,0,1, 1,0, 0,  1,1,1, 1,1, 0,
    1,1,1, 1,1, 0,  0,1,1, 0,1, 0,  0,0,1, 0,0, 0,
};
static const float FACE_BACK[] = {
    1,0,0, 1,0, 0,  0,0,0, 0,0, 0,  0,1,0, 0,1, 0,
    0,1,0, 0,1, 0,  1,1,0, 1,1, 0,  1,0,0, 1,0, 0,
};

// ─────────────────────────────────────────────────────────────────────────────
// Construtor / Destrutor
// ─────────────────────────────────────────────────────────────────────────────
Chunk::Chunk(glm::vec3 pos, Shader& shader) : position(pos) {
    generateBlocks();
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    // Nota: luz é inicializada pelo World após todos os chunks serem criados
}

Chunk::~Chunk() {
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
}

// ─────────────────────────────────────────────────────────────────────────────
// Blocos
// ─────────────────────────────────────────────────────────────────────────────
void Chunk::generateBlocks() {
    static TerrainGenerator terrainGen(1337);
    terrainGen.generateChunkBlocks(
        &blocks[0][0][0],
        (int)position.x, (int)position.y, (int)position.z,
        SIZE_X, SIZE_Y, SIZE_Z
    );
}

bool Chunk::isAir(int x, int y, int z) const {
    if (!inBounds(x, y, z)) return true;
    return blocks[x][y][z] == BLOCK_AIR;
}

BlockType Chunk::getBlock(int x, int y, int z) const {
    if (!inBounds(x, y, z)) return BLOCK_AIR;
    return blocks[x][y][z];
}

void Chunk::setBlock(int x, int y, int z, BlockType type) {
    if (!inBounds(x, y, z)) return;
    blocks[x][y][z] = type;
}

// ─────────────────────────────────────────────────────────────────────────────
// Luz — getters/setters
// lightMap[x][y][z]: bits 7-4 = skylight, bits 3-0 = blocklight
// ─────────────────────────────────────────────────────────────────────────────
int Chunk::getSkyLight(int x, int y, int z) const {
    if (!inBounds(x, y, z)) return 0;
    return (lightMap[x][y][z] >> 4) & 0xF;
}

int Chunk::getBlockLight(int x, int y, int z) const {
    if (!inBounds(x, y, z)) return 0;
    return lightMap[x][y][z] & 0xF;
}

void Chunk::setSkyLight(int x, int y, int z, int val) {
    if (!inBounds(x, y, z)) return;
    val = val < 0 ? 0 : (val > 15 ? 15 : val);
    lightMap[x][y][z] = (uint8_t)((lightMap[x][y][z] & 0x0F) | (val << 4));
}

void Chunk::setBlockLight(int x, int y, int z, int val) {
    if (!inBounds(x, y, z)) return;
    val = val < 0 ? 0 : (val > 15 ? 15 : val);
    lightMap[x][y][z] = (uint8_t)((lightMap[x][y][z] & 0xF0) | val);
}

float Chunk::getLightValue(int x, int y, int z) const {
    if (!inBounds(x, y, z)) return 1.0f; // fora dos limites = iluminado (borda)
    int sky   = (lightMap[x][y][z] >> 4) & 0xF;
    int block = lightMap[x][y][z] & 0xF;
    int best  = sky > block ? sky : block;
    return (float)best / (float)MAX_LIGHT;
}

void Chunk::clearLight() {
    for (int x = 0; x < SIZE_X; x++)
    for (int y = 0; y < SIZE_Y; y++)
    for (int z = 0; z < SIZE_Z; z++)
        lightMap[x][y][z] = 0;
}

// ─────────────────────────────────────────────────────────────────────────────
// initSkyLight
//
// Propaga skylight de cima para baixo.
// Coluna livre de cima → y máximo recebe SKY_LIGHT_MAX.
// Blocos opacos bloqueiam completamente (skylight = 0 abaixo).
// Blocos transparentes (folha, água) reduzem em 1 por bloco atravessado.
//
// worldGetSkyLight: para colunas que entram pelo topo vindo de fora
//   (chunks acima — no seu engine o mundo é flat, SIZE_Y é o total,
//    então o topo do chunk é sempre céu aberto)
// ─────────────────────────────────────────────────────────────────────────────
void Chunk::initSkyLight(std::function<int(int,int,int)> worldGetSkyLight) {
    // Varre cada coluna XZ
    for (int x = 0; x < SIZE_X; x++)
    for (int z = 0; z < SIZE_Z; z++) {
        int currentLight = SKY_LIGHT_MAX;

        // De cima para baixo
        for (int y = SIZE_Y - 1; y >= 0; y--) {
            BlockType bt = blocks[x][y][z];

            if (isOpaque(bt)) {
                // Bloco sólido: zera skylight aqui e em tudo abaixo
                currentLight = 0;
                setSkyLight(x, y, z, 0);
            } else {
                // Ar, água, folha: propaga mas folha/água atenuam em 1
                if (bt == BLOCK_LEAF || bt == BLOCK_WATER) {
                    currentLight = currentLight > 0 ? currentLight - 1 : 0;
                }
                setSkyLight(x, y, z, currentLight);
            }
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// propagateLight
//
// BFS que expande tanto skylight quanto blocklight para os 6 vizinhos.
// Cada passo reduz a luz em 1.
//
// Semeia a fila com:
//   - Todos os voxels com skylight > 0 (depois do initSkyLight)
//   - Todos os voxels com blocklight > 0 (blocos emissivos)
//
// Para vizinhos fora dos limites, consulta worldGetBlock e worldGetBlockLight
// para não perder luz que vem dos chunks vizinhos.
// ─────────────────────────────────────────────────────────────────────────────
void Chunk::propagateLight(
    std::function<BlockType(int,int,int)> worldGetBlock,
    std::function<int(int,int,int)>       worldGetSkyLight)
{
    const int dx[] = {1,-1, 0, 0, 0, 0};
    const int dy[] = {0, 0, 1,-1, 0, 0};
    const int dz[] = {0, 0, 0, 0, 1,-1};

    // Retorna bloco (local ou vizinho)
    auto getBlock_ = [&](int x, int y, int z) -> BlockType {
        if (inBounds(x, y, z)) return blocks[x][y][z];
        if (worldGetBlock) return worldGetBlock(
            (int)position.x + x,
            (int)position.y + y,
            (int)position.z + z);
        return BLOCK_AIR;
    };

    // Retorna skylight de vizinho externo
    auto getNeighborSkyLight = [&](int x, int y, int z) -> int {
        if (!worldGetSkyLight) return 0;
        return worldGetSkyLight(
            (int)position.x + x,
            (int)position.y + y,
            (int)position.z + z);
    };

    // ── BFS Skylight ──────────────────────────────────────────────────────
    // Semeia: (1) voxels internos com skylight > 0 após initSkyLight
    //         (2) voxels de borda que recebem luz do chunk vizinho
    {
        std::queue<std::tuple<int,int,int>> q;

        // (1) Semeia internos
        for (int x = 0; x < SIZE_X; x++)
        for (int y = 0; y < SIZE_Y; y++)
        for (int z = 0; z < SIZE_Z; z++) {
            if (getSkyLight(x, y, z) > 0)
                q.push({x, y, z});
        }

        // (2) Semeia pelas bordas com luz real dos vizinhos
        // Para cada face do cubo (borda X=0, X=SIZE_X-1, Z=0, Z=SIZE_Z-1)
        // verifica se há luz vindo de fora e injeta no voxel de borda
        if (worldGetSkyLight) {
            // Borda X=0: vizinho está em x=-1
            for (int y = 0; y < SIZE_Y; y++)
            for (int z = 0; z < SIZE_Z; z++) {
                if (isOpaque(blocks[0][y][z])) continue;
                int neighborLight = getNeighborSkyLight(-1, y, z);
                int inject = neighborLight - 1;
                if (inject > getSkyLight(0, y, z)) {
                    setSkyLight(0, y, z, inject);
                    q.push({0, y, z});
                }
            }
            // Borda X=SIZE_X-1: vizinho em x=SIZE_X
            for (int y = 0; y < SIZE_Y; y++)
            for (int z = 0; z < SIZE_Z; z++) {
                if (isOpaque(blocks[SIZE_X-1][y][z])) continue;
                int neighborLight = getNeighborSkyLight(SIZE_X, y, z);
                int inject = neighborLight - 1;
                if (inject > getSkyLight(SIZE_X-1, y, z)) {
                    setSkyLight(SIZE_X-1, y, z, inject);
                    q.push({SIZE_X-1, y, z});
                }
            }
            // Borda Z=0: vizinho em z=-1
            for (int x = 0; x < SIZE_X; x++)
            for (int y = 0; y < SIZE_Y; y++) {
                if (isOpaque(blocks[x][y][0])) continue;
                int neighborLight = getNeighborSkyLight(x, y, -1);
                int inject = neighborLight - 1;
                if (inject > getSkyLight(x, y, 0)) {
                    setSkyLight(x, y, 0, inject);
                    q.push({x, y, 0});
                }
            }
            // Borda Z=SIZE_Z-1: vizinho em z=SIZE_Z
            for (int x = 0; x < SIZE_X; x++)
            for (int y = 0; y < SIZE_Y; y++) {
                if (isOpaque(blocks[x][y][SIZE_Z-1])) continue;
                int neighborLight = getNeighborSkyLight(x, y, SIZE_Z);
                int inject = neighborLight - 1;
                if (inject > getSkyLight(x, y, SIZE_Z-1)) {
                    setSkyLight(x, y, SIZE_Z-1, inject);
                    q.push({x, y, SIZE_Z-1});
                }
            }
        }

        // BFS interno
        while (!q.empty()) {
            auto [x, y, z] = q.front(); q.pop();
            int curLight = getSkyLight(x, y, z);
            if (curLight <= 1) continue;

            for (int i = 0; i < 6; i++) {
                int nx = x + dx[i];
                int ny = y + dy[i];
                int nz = z + dz[i];
                if (!inBounds(nx, ny, nz)) continue; // não sai do chunk neste BFS

                BlockType nbt = blocks[nx][ny][nz];
                if (isOpaque(nbt)) continue;

                // Propagação vertical para baixo sem atenuação se vem do céu
                int newLight = (i == 3 && curLight == SKY_LIGHT_MAX)
                               ? SKY_LIGHT_MAX
                               : curLight - 1;

                if (nbt == BLOCK_LEAF || nbt == BLOCK_WATER)
                    newLight = newLight > 0 ? newLight - 1 : 0;

                if (newLight > getSkyLight(nx, ny, nz)) {
                    setSkyLight(nx, ny, nz, newLight);
                    q.push({nx, ny, nz});
                }
            }
        }
    }

    // ── BFS Blocklight ────────────────────────────────────────────────────
    {
        std::queue<std::tuple<int,int,int>> q;

        for (int x = 0; x < SIZE_X; x++)
        for (int y = 0; y < SIZE_Y; y++)
        for (int z = 0; z < SIZE_Z; z++) {
            int emission = getLightEmission(blocks[x][y][z]);
            if (emission > 0) {
                setBlockLight(x, y, z, emission);
                q.push({x, y, z});
            }
        }

        while (!q.empty()) {
            auto [x, y, z] = q.front(); q.pop();
            int curLight = getBlockLight(x, y, z);
            if (curLight <= 1) continue;

            for (int i = 0; i < 6; i++) {
                int nx = x + dx[i];
                int ny = y + dy[i];
                int nz = z + dz[i];
                if (!inBounds(nx, ny, nz)) continue;

                BlockType nbt = blocks[nx][ny][nz];
                if (isOpaque(nbt)) continue;

                int newLight = curLight - 1;
                if (nbt == BLOCK_LEAF || nbt == BLOCK_WATER)
                    newLight = newLight > 0 ? newLight - 1 : 0;

                if (newLight > getBlockLight(nx, ny, nz)) {
                    setBlockLight(nx, ny, nz, newLight);
                    q.push({nx, ny, nz});
                }
            }
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// getLightForFace
//
// Retorna a luz do voxel vizinho na direção da face.
// Se estiver fora do chunk, usa worldGetLight.
// ─────────────────────────────────────────────────────────────────────────────
static float getLightForFace(
    const Chunk* chunk, int x, int y, int z,
    std::function<float(int,int,int)> worldGetLight)
{
    if (chunk->inBounds(x, y, z))  // inBounds é private — usamos getLightValue que já faz bounds check
        return chunk->getLightValue(x, y, z);
    if (worldGetLight)
        return worldGetLight(
            (int)chunk->position.x + x,
            (int)chunk->position.y + y,
            (int)chunk->position.z + z);
    return 1.0f; // sem worldGetLight = borda iluminada
}

// ─────────────────────────────────────────────────────────────────────────────
// generateMesh
//
// Agora passa lightValue (do lightMap) junto com lightFactor (baked por face).
// No shader: finalLight = ambientMin + lightValue * sunIntensity * lightFactor
// ─────────────────────────────────────────────────────────────────────────────
void Chunk::generateMesh(
    std::function<bool(int,int,int)>  worldIsAir,
    std::function<float(int,int,int)> worldGetLight)
{
    vertices.clear();

    for (int x = 0; x < SIZE_X; x++)
    for (int y = 0; y < SIZE_Y; y++)
    for (int z = 0; z < SIZE_Z; z++) {
        BlockType type = blocks[x][y][z];
        if (type == BLOCK_AIR) continue;

        const BlockDef& def = BlockRegistry::get(type);

        auto neighborIsAir = [&](int lx, int ly, int lz) -> bool {
            if (inBounds(lx, ly, lz)) return isAir(lx, ly, lz);
            if (worldIsAir) return worldIsAir(
                (int)position.x + lx,
                (int)position.y + ly,
                (int)position.z + lz);
            return true;
        };

        // Luz do voxel do lado de cada face (o voxel que "vê" a face)
        auto faceLight = [&](int lx, int ly, int lz) -> float {
            return getLightForFace(this, lx, ly, lz, worldGetLight);
        };

        if (neighborIsAir(x+1, y,   z  )) addFace(FACE_RIGHT,  x,y,z, (float)def.sideLayer, LF_RIGHT,  faceLight(x+1, y,   z  ));
        if (neighborIsAir(x-1, y,   z  )) addFace(FACE_LEFT,   x,y,z, (float)def.sideLayer, LF_LEFT,   faceLight(x-1, y,   z  ));
        if (neighborIsAir(x,   y+1, z  )) addFace(FACE_TOP,    x,y,z, (float)def.topLayer,  LF_TOP,    faceLight(x,   y+1, z  ));
        if (neighborIsAir(x,   y-1, z  )) addFace(FACE_BOTTOM, x,y,z, (float)def.botLayer,  LF_BOTTOM, faceLight(x,   y-1, z  ));
        if (neighborIsAir(x,   y,   z+1)) addFace(FACE_FRONT,  x,y,z, (float)def.sideLayer, LF_FRONT,  faceLight(x,   y,   z+1));
        if (neighborIsAir(x,   y,   z-1)) addFace(FACE_BACK,   x,y,z, (float)def.sideLayer, LF_BACK,   faceLight(x,   y,   z-1));
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// uploadMesh — STRIDE=8: [x,y,z, u,v, texLayer, lightFactor, lightValue]
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

    // attrib 3: lightFactor (baked por face)
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE,
                          STRIDE * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(3);

    // attrib 4: lightValue (do lightMap, [0,1])  ← novo
    glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE,
                          STRIDE * sizeof(float), (void*)(7 * sizeof(float)));
    glEnableVertexAttribArray(4);

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
// addFace — STRIDE=8: [x,y,z, u,v, texLayer, lightFactor, lightValue]
// ─────────────────────────────────────────────────────────────────────────────
void Chunk::addFace(const float* face, int x, int y, int z,
                    float texLayer, float lightFactor, float lightValue)
{
    for (int i = 0; i < 6 * 6; i += 6) {
        vertices.push_back(face[i + 0] + x);
        vertices.push_back(face[i + 1] + y);
        vertices.push_back(face[i + 2] + z);
        vertices.push_back(face[i + 3]);     // u
        vertices.push_back(face[i + 4]);     // v
        vertices.push_back(texLayer);
        vertices.push_back(lightFactor);
        vertices.push_back(lightValue);      // novo
    }
}

} // namespace fractal_engine::world