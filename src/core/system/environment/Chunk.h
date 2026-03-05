#pragma once
#include <vector>
#include <functional>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "../shader/Shader.h"
#include "SimpleNoise.h"

#include "../texture/TextureLoader.h"

// Tipos de bloco
enum BlockType : uint8_t {
    BLOCK_AIR   = 0,
    BLOCK_STONE = 1,
    BLOCK_DIRT  = 2,
    BLOCK_GRASS = 3,
};

class Chunk {
public:
    static constexpr int SIZE_X = 16;
    static constexpr int SIZE_Y = 64;  // altura máxima do chunk
    static constexpr int SIZE_Z = 16;
    static constexpr int STRIDE = 6;   // x,y,z,u,v,faceType

    glm::vec3 position;  // posição de mundo (canto inferior esquerdo)

    Chunk(glm::vec3 pos, Shader& shader);
    ~Chunk();

    // Geração de blocos com noise
    void generateBlocks();

    // Gera a mesh. Recebe um callback opcional para checar blocos fora do chunk (vizinhos)
    // worldIsAir(worldX, worldY, worldZ) -> true se o bloco é ar
    void generateMesh(std::function<bool(int,int,int)> worldIsAir = nullptr);

    // Envia a mesh atual para a GPU (chame após generateMesh)
    void uploadMesh();

    void Draw(Shader& shader);

    // Retorna se a posição LOCAL é ar (também retorna true para fora dos limites)
    bool isAir(int x, int y, int z) const;

    BlockType getBlock(int x, int y, int z) const;

private:
    BlockType blocks[SIZE_X][SIZE_Y][SIZE_Z];
    std::vector<float> vertices;

    GLuint vao = 0, vbo = 0;
    GLuint textures[4]; // top, bottom, side_stone, side_dirt

    SimpleNoise noise{1337};

    void addFace(const float* face, int x, int y, int z, float faceType);
    //void TextureLoader(GLuint& tex, const char* path);
};