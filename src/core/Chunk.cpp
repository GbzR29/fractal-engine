#include "../headers/Chunk.h"

// each vertex: pos(3) uv(2) texID(1)
static constexpr int STRIDE = 6;

// ===============================================
// ALL FACES IN CCW
// glFrontFace(GL_CCW) + glCullFace(GL_BACK)
// ===============================================

// -------- FACE RIGHT (+X)
static const float FACE_RIGHT[] = {
    // tri 1
    0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 2,
    0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 2,
    0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 2,
    // tri 2
    0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 2,
    0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 2,
    0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 2,
};

// -------- FACE LEFT (-X)
static const float FACE_LEFT[] = {
    // tri 1
   -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 2,
   -0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 2,
   -0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 2,
    // tri 2
   -0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 2,
   -0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 2,
   -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 2,
};

// -------- FACE TOP (+Y)
static const float FACE_TOP[] = {
    // tri 1
   -0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 0,
    0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0,
    0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0,
    // tri 2
    0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0,
   -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0,
   -0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 0,
};

// -------- FACE BOTTOM (-Y)
static const float FACE_BOTTOM[] = {
    // tri 1
   -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 1,
    0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 1,
    0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 1,
    // tri 2
    0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 1,
   -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 1,
   -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 1,
};

// -------- FACE FRONT (+Z)
static const float FACE_FRONT[] = {
    // tri 1
   -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 2,
    0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 2,
    0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 2,
    // tri 2
    0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 2,
   -0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 2,
   -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 2,
};

// -------- FACE BACK (-Z)
static const float FACE_BACK[] = {
    // tri 1
    0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 2,
   -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 2,
   -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 2,
    // tri 2
   -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 2,
    0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 2,
    0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 2,
};

Chunk::Chunk(glm::vec3 pos, Shader& shader) : position(pos)
{
    generateBlocks();
    generateMesh();

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glGenTextures(3, textures);   

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);    

    //Position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, STRIDE * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    //texture coord
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, STRIDE * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    //texture ID
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, STRIDE * sizeof(float), (void*)(5 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);

    TextureLoader(textures[0], "../src/assets/grass-top.jpg");
    TextureLoader(textures[1], "../src/assets/grass-bottom.jpg");
    TextureLoader(textures[2], "../src/assets/grass_side.png");

    shader.use();
    shader.setInt("texture1", 0);
    shader.setInt("texture2", 1);
    shader.setInt("texture3", 2);
}

Chunk::~Chunk() {
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
}

void Chunk::generateBlocks() {

    for (int x = 0; x < SIZE_X; x++) {
        for (int z = 0; z < SIZE_Z; z++) {

            // Global position of the world
            int worldX = position.x + x;
            int worldZ = position.z + z;            

            float n = noise.fractalNoise(worldX, worldZ);
            int height = (int)(n * (SIZE_Y - 1));            

            for (int y = 0; y < SIZE_Y; y++) {
                blocks[x][y][z] = (y <= height);
            }
        }
    }
}

// Return true if the position must be considerated AIR 
bool Chunk::isAir(int x, int y, int z) const {

    if (x < 0 || y < 0 || z < 0 || x >= SIZE_X || y >= SIZE_Y || z >= SIZE_Z){
        return true;
    }

    //Question: The block in this position DON'T exsist? If not, then return true
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
    for (int x = 0; x < SIZE_X; x++){

        for (int y = 0; y < SIZE_Y; y++){

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

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textures[0]);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, textures[1]);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, textures[2]);   

    glm::mat4 model(1.0f);
    model = glm::translate(model, position);
    shader.setMat4("model", model);

    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, vertices.size() / STRIDE);
    glBindVertexArray(0);
}

glm::vec3 Chunk::getPosition(){
    return position;
}

