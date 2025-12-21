#pragma once

#include <vector>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Shader.h"

#include "SimpleNoise.h"

#include "../headers/TextureLoader.h"

class Chunk {
public:
    static constexpr int SIZE_X = 16, SIZE_Y = 16, SIZE_Z = 16;

    Chunk(glm::vec3 position, Shader& shader);
    ~Chunk();

    void Draw(Shader& shader);

    glm::vec3 getPosition();

private:
    SimpleNoise noise;

    bool blocks[SIZE_X][SIZE_Y][SIZE_Z];
    std::vector<float> vertices;

    std::vector<Chunk> chunks;

    GLuint vao, vbo;
    GLuint textures[4];

    glm::vec3 position;

    void generateBlocks();
    void generateMesh();
    void addFace(const float* face, int x, int y, int z);
    bool isAir(int x, int y, int z) const;
};
