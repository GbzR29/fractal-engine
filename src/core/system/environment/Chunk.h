#pragma once

#include <vector>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "../shader/Shader.h"
#include "SimpleNoise.h"
#include "../texture/TextureLoader.h"

class Chunk {
public:
    // Constantes de tamanho acessíveis para o World
    static constexpr int SIZE_X = 16, SIZE_Y = 16, SIZE_Z = 16;
    static constexpr int STRIDE = 6;

    Chunk(glm::vec3 position, Shader& shader);
    ~Chunk();

    void Draw(Shader& shader);
    
    // Agora pública para que o World e o Player possam checar colisões
    bool isAir(int x, int y, int z) const;
    
    glm::vec3 getPosition() const { return position; }

private:
    SimpleNoise noise;
    bool blocks[SIZE_X][SIZE_Y][SIZE_Z];
    std::vector<float> vertices;

    GLuint vao, vbo;
    GLuint textures[3]; // Reduzido para 3 conforme o uso atual

    glm::vec3 position;

    void generateBlocks();
    void generateMesh();
    void addFace(const float* face, int x, int y, int z);
};