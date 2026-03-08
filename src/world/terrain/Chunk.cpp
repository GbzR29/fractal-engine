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
    glGenVertexArrays(1, &vao);  glGenBuffers(1, &vbo);
    glGenVertexArrays(1, &vaoT); glGenBuffers(1, &vboT);
}

Chunk::~Chunk() {
    glDeleteVertexArrays(1, &vao);  glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vaoT); glDeleteBuffers(1, &vboT);
}

// ─────────────────────────────────────────────────────────────────────────────
// Blocos
// ─────────────────────────────────────────────────────────────────────────────
void Chunk::generateBlocks() {
    static TerrainGenerator fallbackGen(1337);
    generateBlocks(fallbackGen);
}

void Chunk::generateBlocks(const TerrainGenerator& gen) {
    gen.generateChunkBlocks(
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
// sampleLight — amostra luz em posição local (ou mundo via worldGetLight)
// ─────────────────────────────────────────────────────────────────────────────
static float sampleLight(
    const Chunk* chunk, int lx, int ly, int lz,
    std::function<float(int,int,int)> worldGetLight)
{
    if (chunk->inBounds(lx, ly, lz))
        return chunk->getLightValue(lx, ly, lz);
    if (worldGetLight)
        return worldGetLight(
            (int)chunk->position.x + lx,
            (int)chunk->position.y + ly,
            (int)chunk->position.z + lz);
    return 1.0f;
}

// ─────────────────────────────────────────────────────────────────────────────
// cornerLight — smooth lighting para um canto de vértice
//
// Para uma face na direção (nx, ny, nz) relativa ao bloco (x,y,z),
// o vértice no canto (ox, oy, oz) — onde cada componente é 0 ou 1 —
// toca até 4 voxels de ar naquele plano.
//
// Algoritmo padrão (Minecraft):
//   side1, side2 = os dois vizinhos axiais do canto no plano da face
//   corner       = o vizinho diagonal
//   Se side1 E side2 são opacos → corner não contribui (AO completo)
//   valor = (center + side1 + side2 + corner) / 4
// ─────────────────────────────────────────────────────────────────────────────
static float cornerLight(
    const Chunk* chunk,
    int x, int y, int z,           // posição do bloco
    int nx, int ny, int nz,        // direção da face (+1 ou -1 por eixo)
    int s1x, int s1y, int s1z,    // primeiro vizinho lateral no plano
    int s2x, int s2y, int s2z,    // segundo vizinho lateral no plano
    std::function<float(int,int,int)> worldGetLight)
{
    // Posição do voxel de ar na frente da face
    int fx = x + nx, fy = y + ny, fz = z + nz;

    float center = sampleLight(chunk, fx,          fy,          fz,          worldGetLight);
    float side1  = sampleLight(chunk, fx + s1x,    fy + s1y,    fz + s1z,    worldGetLight);
    float side2  = sampleLight(chunk, fx + s2x,    fy + s2y,    fz + s2z,    worldGetLight);
    float corner = sampleLight(chunk, fx + s1x+s2x, fy + s1y+s2y, fz + s1z+s2z, worldGetLight);

    // Se ambos os lados são opacos, o canto está em AO total — corner não contribui
    bool side1Opaque = !chunk->inBounds(fx+s1x, fy+s1y, fz+s1z)
                       ? false
                       : isOpaque(chunk->getBlock(fx+s1x, fy+s1y, fz+s1z));
    bool side2Opaque = !chunk->inBounds(fx+s2x, fy+s2y, fz+s2z)
                       ? false
                       : isOpaque(chunk->getBlock(fx+s2x, fy+s2y, fz+s2z));

    if (side1Opaque && side2Opaque)
        return (center + side1 + side2) / 3.0f;

    return (center + side1 + side2 + corner) / 4.0f;
}

// ─────────────────────────────────────────────────────────────────────────────
// generateMesh — smooth lighting por vértice
//
// Cada face tem 4 cantos únicos. Os 6 vértices do quad (2 triângulos)
// recebem a luz calculada por canto via cornerLight().
//
// STRIDE=8: [x, y, z, u, v, texLayer, lightFactor, lightValue_por_vertice]
// ─────────────────────────────────────────────────────────────────────────────
void Chunk::generateMesh(
    std::function<bool(int,int,int)>  worldIsAir,
    std::function<float(int,int,int)> worldGetLight)
{
    vertices.clear();
    verticesTransparent.clear();

    auto neighborIsAir = [&](int lx, int ly, int lz) -> bool {
        if (inBounds(lx, ly, lz)) return isAir(lx, ly, lz);
        if (worldIsAir) return worldIsAir(
            (int)position.x + lx,
            (int)position.y + ly,
            (int)position.z + lz);
        return true;
    };

    auto light = [&](int lx, int ly, int lz) -> float {
        return sampleLight(this, lx, ly, lz, worldGetLight);
    };

    auto corner = [&](int x, int y, int z,
                      int nx, int ny, int nz,
                      int s1x, int s1y, int s1z,
                      int s2x, int s2y, int s2z) -> float {
        return cornerLight(this, x, y, z, nx, ny, nz,
                           s1x, s1y, s1z, s2x, s2y, s2z, worldGetLight);
    };

    for (int x = 0; x < SIZE_X; x++)
    for (int y = 0; y < SIZE_Y; y++)
    for (int z = 0; z < SIZE_Z; z++) {
        BlockType type = blocks[x][y][z];
        if (type == BLOCK_AIR) continue;

        const BlockDef& def = BlockRegistry::get(type);

        // Blocos transparentes (folhas/água) vão pro buffer separado
        bool useTransparent = isTransparent(type) && type != BLOCK_AIR;
        auto& targetVerts   = useTransparent ? verticesTransparent : vertices;

        // Para folhas: gera face se vizinho for ar ou água (não outra folha)
        // Isso resolve o problema de faces invisíveis entre dois blocos de folha
        auto shouldDrawFace = [&](int lx, int ly, int lz) -> bool {
            if (isLeafLike(type)) {
                // Gera face se vizinho não for do mesmo tipo de folha
                BlockType nb = inBounds(lx,ly,lz) ? blocks[lx][ly][lz]
                    : (worldIsAir ? (worldIsAir((int)position.x+lx,(int)position.y+ly,(int)position.z+lz) ? BLOCK_AIR : BLOCK_STONE) : BLOCK_AIR);
                return nb == BLOCK_AIR || nb == BLOCK_WATER;
            }
            if (type == BLOCK_WATER) {
                BlockType nb = inBounds(lx,ly,lz) ? blocks[lx][ly][lz]
                    : (worldIsAir ? (worldIsAir((int)position.x+lx,(int)position.y+ly,(int)position.z+lz) ? BLOCK_AIR : BLOCK_STONE) : BLOCK_AIR);
                return nb == BLOCK_AIR;
            }
            return neighborIsAir(lx, ly, lz);
        };

        // Helper para emitir face no buffer correto
        auto emit = [&](const float* face, float texLayer, float lf,
                        float l0, float l1, float l2, float l3, float l4, float l5) {
            const float lights[6] = {l0,l1,l2,l3,l4,l5};
            for (int v = 0; v < 6; v++) {
                int i = v * 6;
                targetVerts.push_back(face[i+0]+x); targetVerts.push_back(face[i+1]+y);
                targetVerts.push_back(face[i+2]+z); targetVerts.push_back(face[i+3]);
                targetVerts.push_back(face[i+4]);   targetVerts.push_back(texLayer);
                targetVerts.push_back(lf);          targetVerts.push_back(lights[v]);
            }
        };

        // ── FACE RIGHT (nx=+1) ────────────────────────────────────────────
        if (shouldDrawFace(x+1, y, z)) {
            // 4 cantos únicos do quad
            float c00 = corner(x,y,z, +1,0,0,  0,-1,0,  0,0,-1); // (1,0,0)
            float c10 = corner(x,y,z, +1,0,0,  0,+1,0,  0,0,-1); // (1,1,0)
            float c11 = corner(x,y,z, +1,0,0,  0,+1,0,  0,0,+1); // (1,1,1)
            float c01 = corner(x,y,z, +1,0,0,  0,-1,0,  0,0,+1); // (1,0,1)
            // Quad: triângulo 1 = c00,c10,c11 | triângulo 2 = c11,c01,c00
            emit(FACE_RIGHT,(float)def.sideLayer,LF_RIGHT, c00,c10,c11,c11,c01,c00);
        }

        // ── FACE LEFT (nx=-1) ─────────────────────────────────────────────
        // Vértices: (0,0,1),(0,1,1),(0,1,0),(0,0,0)
        if (shouldDrawFace(x-1, y, z)) {
            float c01 = corner(x,y,z, -1,0,0,  0,-1,0,  0,0,+1); // (0,0,1)
            float c11 = corner(x,y,z, -1,0,0,  0,+1,0,  0,0,+1); // (0,1,1)
            float c10 = corner(x,y,z, -1,0,0,  0,+1,0,  0,0,-1); // (0,1,0)
            float c00 = corner(x,y,z, -1,0,0,  0,-1,0,  0,0,-1); // (0,0,0)
            emit(FACE_LEFT,(float)def.sideLayer,LF_LEFT, c01,c11,c10,c10,c00,c01);
        }

        // ── FACE TOP (ny=+1) ──────────────────────────────────────────────
        // Vértices: (0,1,1),(1,1,1),(1,1,0),(0,1,0)
        if (shouldDrawFace(x, y+1, z)) {
            float c01 = corner(x,y,z, 0,+1,0,  -1,0,0,  0,0,+1); // (0,1,1)
            float c11 = corner(x,y,z, 0,+1,0,  +1,0,0,  0,0,+1); // (1,1,1)
            float c10 = corner(x,y,z, 0,+1,0,  +1,0,0,  0,0,-1); // (1,1,0)
            float c00 = corner(x,y,z, 0,+1,0,  -1,0,0,  0,0,-1); // (0,1,0)
            emit(FACE_TOP,(float)def.topLayer,LF_TOP, c01,c11,c10,c10,c00,c01);
        }

        // ── FACE BOTTOM (ny=-1) ───────────────────────────────────────────
        // Vértices: (0,0,0),(1,0,0),(1,0,1),(0,0,1)
        if (shouldDrawFace(x, y-1, z)) {
            float c00 = corner(x,y,z, 0,-1,0,  -1,0,0,  0,0,-1); // (0,0,0)
            float c10 = corner(x,y,z, 0,-1,0,  +1,0,0,  0,0,-1); // (1,0,0)
            float c11 = corner(x,y,z, 0,-1,0,  +1,0,0,  0,0,+1); // (1,0,1)
            float c01 = corner(x,y,z, 0,-1,0,  -1,0,0,  0,0,+1); // (0,0,1)
            emit(FACE_BOTTOM,(float)def.botLayer,LF_BOTTOM, c00,c10,c11,c11,c01,c00);
        }

        // ── FACE FRONT (nz=+1) ───────────────────────────────────────────
        // Vértices: (0,0,1),(1,0,1),(1,1,1),(0,1,1)
        if (shouldDrawFace(x, y, z+1)) {
            float c00 = corner(x,y,z, 0,0,+1,  -1,0,0,  0,-1,0); // (0,0,1)
            float c10 = corner(x,y,z, 0,0,+1,  +1,0,0,  0,-1,0); // (1,0,1)
            float c11 = corner(x,y,z, 0,0,+1,  +1,0,0,  0,+1,0); // (1,1,1)
            float c01 = corner(x,y,z, 0,0,+1,  -1,0,0,  0,+1,0); // (0,1,1)
            emit(FACE_FRONT,(float)def.sideLayer,LF_FRONT, c00,c10,c11,c11,c01,c00);
        }

        // ── FACE BACK (nz=-1) ────────────────────────────────────────────
        // Vértices: (1,0,0),(0,0,0),(0,1,0),(1,1,0)
        if (shouldDrawFace(x, y, z-1)) {
            float c10 = corner(x,y,z, 0,0,-1,  +1,0,0,  0,-1,0); // (1,0,0)
            float c00 = corner(x,y,z, 0,0,-1,  -1,0,0,  0,-1,0); // (0,0,0)
            float c01 = corner(x,y,z, 0,0,-1,  -1,0,0,  0,+1,0); // (0,1,0)
            float c11 = corner(x,y,z, 0,0,-1,  +1,0,0,  0,+1,0); // (1,1,0)
            emit(FACE_BACK,(float)def.sideLayer,LF_BACK, c10,c00,c01,c01,c11,c10);
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// uploadMesh — STRIDE=8: [x,y,z, u,v, texLayer, lightFactor, lightValue]
// lightValue agora é por vértice (smooth lighting)
// ─────────────────────────────────────────────────────────────────────────────
static void setupVAO(GLuint vao, GLuint vbo,
                     const std::vector<float>& verts, int stride) {
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, verts.size()*sizeof(float),
                 verts.data(), GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,stride*sizeof(float),(void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,stride*sizeof(float),(void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2,1,GL_FLOAT,GL_FALSE,stride*sizeof(float),(void*)(5*sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(3,1,GL_FLOAT,GL_FALSE,stride*sizeof(float),(void*)(6*sizeof(float)));
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(4,1,GL_FLOAT,GL_FALSE,stride*sizeof(float),(void*)(7*sizeof(float)));
    glEnableVertexAttribArray(4);
    glBindVertexArray(0);
}

void Chunk::uploadMesh() {
    setupVAO(vao,  vbo,  vertices,            STRIDE);
    setupVAO(vaoT, vboT, verticesTransparent, STRIDE);
}

void Chunk::DrawOpaque(Shader& shader) {
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

void Chunk::DrawTransparent(Shader& shader) {
    if (verticesTransparent.empty()) return;
    shader.use();
    BlockRegistry::bind(0);
    shader.setInt("texArray", 0);
    glm::mat4 model = glm::translate(glm::mat4(1.0f), position);
    shader.setMat4("model", model);
    glBindVertexArray(vaoT);
    glDrawArrays(GL_TRIANGLES, 0, (GLsizei)(verticesTransparent.size() / STRIDE));
    glBindVertexArray(0);
}

void Chunk::Draw(Shader& shader) {
    DrawOpaque(shader);
    DrawTransparent(shader);
}

// ─────────────────────────────────────────────────────────────────────────────
// addFace — fallback com lightValue uniforme (não usado com smooth lighting)
// ─────────────────────────────────────────────────────────────────────────────
void Chunk::addFace(const float* face, int x, int y, int z,
                    float texLayer, float lightFactor, float lightValue)
{
    // Delega para addFaceSmooth com o mesmo valor em todos os cantos
    addFaceSmooth(face, x, y, z, texLayer, lightFactor,
                  lightValue, lightValue, lightValue,
                  lightValue, lightValue, lightValue);
}

// ─────────────────────────────────────────────────────────────────────────────
// addFaceSmooth — STRIDE=8, luz diferente por vértice
//
// Os 6 floats de luz correspondem aos 6 vértices do quad (2 triângulos):
//   v0,v1,v2  (triângulo 1)
//   v3,v4,v5  (triângulo 2)
// ─────────────────────────────────────────────────────────────────────────────
void Chunk::addFaceSmooth(const float* face, int x, int y, int z,
                           float texLayer, float lightFactor,
                           float l0, float l1, float l2,
                           float l3, float l4, float l5)
{
    const float lights[6] = {l0, l1, l2, l3, l4, l5};
    for (int v = 0; v < 6; v++) {
        int i = v * 6;
        vertices.push_back(face[i + 0] + x);
        vertices.push_back(face[i + 1] + y);
        vertices.push_back(face[i + 2] + z);
        vertices.push_back(face[i + 3]);      // u
        vertices.push_back(face[i + 4]);      // v
        vertices.push_back(texLayer);
        vertices.push_back(lightFactor);
        vertices.push_back(lights[v]);        // luz do canto
    }
}

} // namespace fractal_engine::world