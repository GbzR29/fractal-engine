#include "World.h"
#include <cmath>

void World::addChunk(glm::ivec3 pos, Shader& shader) {
    // Insere um novo chunk no mapa (ex: pos pode ser 0,0,0 ou 16,0,0)
    chunks.emplace(std::piecewise_construct,
                   std::forward_as_tuple(pos),
                   std::forward_as_tuple(glm::vec3(pos), shader));
}

glm::ivec3 World::getChunkCoords(int wx, int wy, int wz) {
    // Cálculo matemático para converter posição de mundo para o início do Chunk
    // SIZE_X e SIZE_Z vêm do seu Chunk.h
    int cx = std::floor((float)wx / Chunk::SIZE_X) * Chunk::SIZE_X;
    int cy = std::floor((float)wy / Chunk::SIZE_Y) * Chunk::SIZE_Y;
    int cz = std::floor((float)wz / Chunk::SIZE_Z) * Chunk::SIZE_Z;
    return glm::ivec3(cx, cy, cz);
}

bool World::isBlockSolid(float wx, float wy, float wz) {
    // 1. Arredonda para baixo para pegar o "quadrado" do grid
    int ix = std::floor(wx);
    int iy = std::floor(wy);
    int iz = std::floor(wz);

    // 2. Encontra qual chunk deveria conter essa posição
    glm::ivec3 chunkKey = getChunkCoords(ix, iy, iz);

    auto it = chunks.find(chunkKey);
    if (it != chunks.end()) {
        // 3. Converte global para local (ex: mundo 17 -> local 1 no chunk 16)
        int lx = ix - chunkKey.x;
        int ly = iy - chunkKey.y;
        int lz = iz - chunkKey.z;

        // 4. Pergunta ao chunk se aquele bloco é ar ou sólido
        return !it->second.isAir(lx, ly, lz);
    }

    // Se o chunk não estiver carregado, tratamos como sólido para 
    // evitar que o player caia no vazio infinito (ou ar, se preferir)
    return false; 
}

void World::render(Shader& shader) {
    for (auto& pair : chunks) {
        pair.second.Draw(shader);
    }
}