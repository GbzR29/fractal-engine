#include "fractal_engine/world/Chunk.h"
#include <iostream>

namespace fractal_engine::world {

// ─────────────────────────────────────────────
// Faces (posição relativa ao centro do bloco)
// Cada vértice: x, y, z, u, v, faceType
// faceType: 0=topo, 1=fundo, 2=lateral
// ─────────────────────────────────────────────
static const float FACE_RIGHT[] = {
    0.5f,-0.5f,-0.5f,  0.0f,0.0f, 2,
    0.5f, 0.5f,-0.5f,  0.0f,1.0f, 2,
    0.5f, 0.5f, 0.5f,  1.0f,1.0f, 2,
    0.5f, 0.5f, 0.5f,  1.0f,1.0f, 2,
    0.5f,-0.5f, 0.5f,  1.0f,0.0f, 2,
    0.5f,-0.5f,-0.5f,  0.0f,0.0f, 2
};
static const float FACE_LEFT[] = {
   -0.5f,-0.5f, 0.5f,  0.0f,0.0f, 2,
   -0.5f, 0.5f, 0.5f,  0.0f,1.0f, 2,
   -0.5f, 0.5f,-0.5f,  1.0f,1.0f, 2,
   -0.5f, 0.5f,-0.5f,  1.0f,1.0f, 2,
   -0.5f,-0.5f,-0.5f,  1.0f,0.0f, 2,
   -0.5f,-0.5f, 0.5f,  0.0f,0.0f, 2
};
static const float FACE_TOP[] = {
   -0.5f, 0.5f, 0.5f,  0.0f,0.0f, 0,
    0.5f, 0.5f, 0.5f,  1.0f,0.0f, 0,
    0.5f, 0.5f,-0.5f,  1.0f,1.0f, 0,
    0.5f, 0.5f,-0.5f,  1.0f,1.0f, 0,
   -0.5f, 0.5f,-0.5f,  0.0f,1.0f, 0,
   -0.5f, 0.5f, 0.5f,  0.0f,0.0f, 0
};
static const float FACE_BOTTOM[] = {
   -0.5f,-0.5f,-0.5f,  0.0f,1.0f, 1,
    0.5f,-0.5f,-0.5f,  1.0f,1.0f, 1,
    0.5f,-0.5f, 0.5f,  1.0f,0.0f, 1,
    0.5f,-0.5f, 0.5f,  1.0f,0.0f, 1,
   -0.5f,-0.5f, 0.5f,  0.0f,0.0f, 1,
   -0.5f,-0.5f,-0.5f,  0.0f,1.0f, 1
};
static const float FACE_FRONT[] = {
   -0.5f,-0.5f, 0.5f,  0.0f,0.0f, 2,
    0.5f,-0.5f, 0.5f,  1.0f,0.0f, 2,
    0.5f, 0.5f, 0.5f,  1.0f,1.0f, 2,
    0.5f, 0.5f, 0.5f,  1.0f,1.0f, 2,
   -0.5f, 0.5f, 0.5f,  0.0f,1.0f, 2,
   -0.5f,-0.5f, 0.5f,  0.0f,0.0f, 2
};
static const float FACE_BACK[] = {
    0.5f,-0.5f,-0.5f,  1.0f,0.0f, 2,
   -0.5f,-0.5f,-0.5f,  0.0f,0.0f, 2,
   -0.5f, 0.5f,-0.5f,  0.0f,1.0f, 2,
   -0.5f, 0.5f,-0.5f,  0.0f,1.0f, 2,
    0.5f, 0.5f,-0.5f,  1.0f,1.0f, 2,
    0.5f,-0.5f,-0.5f,  1.0f,0.0f, 2
};

// ─────────────────────────────────────────────
// Construtor
// ─────────────────────────────────────────────
Chunk::Chunk(glm::vec3 pos, Shader& shader) : position(pos) {
    generateBlocks();
    // Mesh gerada depois pelo World (que pode passar os vizinhos)
    // mas geramos uma primeira passagem sem vizinhos para já ter algo
    generateMesh();

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenTextures(4, textures);

    uploadMesh(); // envia para GPU

    TextureLoader(textures[0], "assets/grass_top.png");
    TextureLoader(textures[1], "assets/dirt.png");
    TextureLoader(textures[2], "assets/grass_side.png");
    TextureLoader(textures[3], "assets/stone.png");

    shader.use();
    shader.setInt("texTop",    0);
    shader.setInt("texBottom", 1);
    shader.setInt("texSide",   2);
}

Chunk::~Chunk() {
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteTextures(4, textures);
}

// ─────────────────────────────────────────────
// Geração de blocos
// ─────────────────────────────────────────────
void Chunk::generateBlocks() {
    for (int x = 0; x < SIZE_X; x++) {
        for (int z = 0; z < SIZE_Z; z++) {
            int worldX = (int)position.x + x;
            int worldZ = (int)position.z + z;

            // Noise retorna [0,1]; mapeamos para altura de terreno
            float n = noise.fractalNoise((float)worldX, (float)worldZ, 80.0f, 5);

            // Altura do terreno: entre 8 e SIZE_Y * 0.75 para variar bem
            int minHeight = 8;
            int maxHeight = (int)(SIZE_Y * 0.75f);
            int surfaceY  = minHeight + (int)(n * (maxHeight - minHeight));

            for (int y = 0; y < SIZE_Y; y++) {
                if (y > surfaceY) {
                    blocks[x][y][z] = BLOCK_AIR;
                } else if (y == surfaceY) {
                    blocks[x][y][z] = BLOCK_GRASS;  // camada de grama
                } else if (y >= surfaceY - 3) {
                    blocks[x][y][z] = BLOCK_DIRT;   // 3 camadas de terra
                } else {
                    blocks[x][y][z] = BLOCK_STONE;  // pedra abaixo
                }
            }
        }
    }
}

// ─────────────────────────────────────────────
// Acessors
// ─────────────────────────────────────────────
bool Chunk::isAir(int x, int y, int z) const {
    if (x < 0 || x >= SIZE_X || y < 0 || y >= SIZE_Y || z < 0 || z >= SIZE_Z)
        return true; // fora dos limites = ar (será tratado pelo callback do World)
    return blocks[x][y][z] == BLOCK_AIR;
}

BlockType Chunk::getBlock(int x, int y, int z) const {
    if (x < 0 || x >= SIZE_X || y < 0 || y >= SIZE_Y || z < 0 || z >= SIZE_Z)
        return BLOCK_AIR;
    return blocks[x][y][z];
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

                // Determina o faceType pelo tipo de bloco
                // topo de grama = 0 (textura de grama), resto lateral = 2
                float topFace  = (blocks[x][y][z] == BLOCK_GRASS) ? 0.0f : 2.0f;
                float botFace  = 1.0f; // fundo sempre usa textura de terra/pedra
                float sideFace = 2.0f;

                // Lambda que checa se um vizinho (em coords locais) é ar,
                // consultando o World para posições fora dos limites do chunk
                auto neighborIsAir = [&](int lx, int ly, int lz) -> bool {
                    // Dentro do chunk: consulta local
                    if (lx >= 0 && lx < SIZE_X &&
                        ly >= 0 && ly < SIZE_Y &&
                        lz >= 0 && lz < SIZE_Z) {
                        return isAir(lx, ly, lz);
                    }
                    // Fora do chunk: converte para coords de mundo e consulta World
                    if (worldIsAir) {
                        int wx = (int)position.x + lx;
                        int wy = (int)position.y + ly;
                        int wz = (int)position.z + lz;
                        return worldIsAir(wx, wy, wz);
                    }
                    return true; // sem callback: expõe a face (pior caso)
                };

                if (neighborIsAir(x+1, y, z)) addFace(FACE_RIGHT,  x, y, z, sideFace);
                if (neighborIsAir(x-1, y, z)) addFace(FACE_LEFT,   x, y, z, sideFace);
                if (neighborIsAir(x, y+1, z)) addFace(FACE_TOP,    x, y, z, topFace);
                if (neighborIsAir(x, y-1, z)) addFace(FACE_BOTTOM, x, y, z, botFace);
                if (neighborIsAir(x, y, z+1)) addFace(FACE_FRONT,  x, y, z, sideFace);
                if (neighborIsAir(x, y, z-1)) addFace(FACE_BACK,   x, y, z, sideFace);
            }
        }
    }
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
                 GL_DYNAMIC_DRAW); // DYNAMIC porque o chunk pode ser remeshado

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
// Helpers
// ─────────────────────────────────────────────
void Chunk::addFace(const float* face, int x, int y, int z, float faceType) {
    for (int i = 0; i < 6 * STRIDE; i += STRIDE) {
        vertices.push_back(face[i + 0] + x);
        vertices.push_back(face[i + 1] + y);
        vertices.push_back(face[i + 2] + z);
        vertices.push_back(face[i + 3]);       // u
        vertices.push_back(face[i + 4]);       // v
        vertices.push_back(faceType);          // sobrescreve o faceType da face com o do bloco
    }
}

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

/*void Chunk::TextureLoader(GLuint& tex, const char* path) {
    // Implemente com stb_image ou similar
    // Exemplo mínimo:
    // stbi_set_flip_vertically_on_load(true);
    // int w,h,ch; unsigned char* data = stbi_load(path,&w,&h,&ch,0);
    // glBindTexture(GL_TEXTURE_2D, tex);
    // glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,w,h,0,ch==4?GL_RGBA:GL_RGB,GL_UNSIGNED_BYTE,data);
    // glGenerateMipmap(GL_TEXTURE_2D);
    // stbi_image_free(data);
}*/

} // namespace fractal_engine::world