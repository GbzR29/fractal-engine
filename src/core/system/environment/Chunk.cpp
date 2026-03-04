#include "Chunk.h"
#include <iostream>

// Definições das faces (mantidas como no seu original)
static const float FACE_RIGHT[] = {
    0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 2,
    0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 2,  
    0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 2,
    0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 2,
    0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 2,
    0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 2
};
static const float FACE_LEFT[] = {
   -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 2,
   -0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 2,
   -0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 2,
   -0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 2,
   -0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 2, 
   -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 2
};
static const float FACE_TOP[] = {
   -0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 0,  
    0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0,  
    0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0,
    0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0, 
   -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0, 
   -0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 0
};
static const float FACE_BOTTOM[] = {
   -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 1,  
    0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 1, 
    0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 1,
    0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 1,
   -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 1,
   -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 1
};
static const float FACE_FRONT[] = {
   -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 2,  
    0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 2, 
    0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 2,
    0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 2,
   -0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 2, 
   -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 2
};
static const float FACE_BACK[] = {
    0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 2, 
   -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 2,
   -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 2,
   -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 2, 
    0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 2,  
    0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 2
};

Chunk::Chunk(glm::vec3 pos, Shader& shader) : position(pos) {
    generateBlocks();
    generateMesh();

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenTextures(3, textures);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    // Atributos do Shader
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, STRIDE * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, STRIDE * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, STRIDE * sizeof(float), (void*)(5 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);

    TextureLoader(textures[0], "assets/grass-top.jpg");
    TextureLoader(textures[1], "assets/grass-bottom.jpg");
    TextureLoader(textures[2], "assets/grass_side.png");

    shader.use();
    shader.setInt("texTop", 0);    
    shader.setInt("texBottom", 1); 
    shader.setInt("texSide", 2);  
}

Chunk::~Chunk() {
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
}

void Chunk::generateBlocks() {
    for (int x = 0; x < SIZE_X; x++) {
        for (int z = 0; z < SIZE_Z; z++) {
            int worldX = (int)position.x + x;
            int worldZ = (int)position.z + z;

            float n = noise.fractalNoise(worldX, worldZ);
            int height = (int)(n * (SIZE_Y - 1));

            for (int y = 0; y < SIZE_Y; y++) {
                blocks[x][y][z] = (y <= height);
            }
        }
    }
}

bool Chunk::isAir(int x, int y, int z) const {
    if (x < 0 || y < 0 || z < 0 || x >= SIZE_X || y >= SIZE_Y || z >= SIZE_Z) {
        return true;
    }
    return !blocks[x][y][z];
}

void Chunk::addFace(const float* face, int x, int y, int z) {
    for (int i = 0; i < 6 * STRIDE; i += STRIDE) {
        vertices.push_back(face[i + 0] + x);
        vertices.push_back(face[i + 1] + y);
        vertices.push_back(face[i + 2] + z);
        vertices.push_back(face[i + 3]);
        vertices.push_back(face[i + 4]);
        vertices.push_back(face[i + 5]);
    }
}

void Chunk::generateMesh() {
    vertices.clear();
    for (int x = 0; x < SIZE_X; x++) {
        for (int y = 0; y < SIZE_Y; y++) {
            for (int z = 0; z < SIZE_Z; z++) {
                if (!blocks[x][y][z]) continue;

                if (isAir(x + 1, y, z)) addFace(FACE_RIGHT, x, y, z);
                if (isAir(x - 1, y, z)) addFace(FACE_LEFT, x, y, z);
                if (isAir(x, y + 1, z)) addFace(FACE_TOP, x, y, z);
                if (isAir(x, y - 1, z)) addFace(FACE_BOTTOM, x, y, z);
                if (isAir(x, y, z + 1)) addFace(FACE_FRONT, x, y, z);
                if (isAir(x, y, z - 1)) addFace(FACE_BACK, x, y, z);
            }
        }
    }
}

void Chunk::Draw(Shader& shader) {
    for (int i = 0; i < 3; i++) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, textures[i]);
    }

    glm::mat4 model = glm::translate(glm::mat4(1.0f), position);
    shader.setMat4("model", model);

    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, (GLsizei)(vertices.size() / STRIDE));
}