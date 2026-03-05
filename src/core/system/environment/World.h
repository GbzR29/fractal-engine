#pragma once
#include <map>
#include <glm/glm.hpp>
#include "Chunk.h"
#include "../shader/Shader.h"

// Comparador para usar glm::ivec3 como chave de map
struct IVec3Cmp {
    bool operator()(const glm::ivec3& a, const glm::ivec3& b) const {
        if (a.x != b.x) return a.x < b.x;
        if (a.y != b.y) return a.y < b.y;
        return a.z < b.z;
    }
};

class World {
public:
    std::map<glm::ivec3, Chunk, IVec3Cmp> chunks;

    // Adiciona um chunk na posição de mundo dada (deve ser múltiplo de SIZE)
    // Após adicionar, remesheia os vizinhos para corrigir bordas
    void addChunk(glm::ivec3 pos, Shader& shader);

    // Gera um grid de chunks ao redor de uma posição central
    // (útil para inicializar o mundo)
    void generateWorld(int radiusX, int radiusZ, Shader& shader);

    // Retorna a chave do chunk que contém a posição de mundo (wx, wy, wz)
    glm::ivec3 getChunkKey(int wx, int wy, int wz) const;

    // Retorna true se o bloco na posição de mundo é sólido
    bool isBlockSolid(float wx, float wy, float wz) const;

    // Retorna true se o bloco na posição de mundo é ar
    bool isAirWorld(int wx, int wy, int wz) const;

    void render(Shader& shader);

private:
    // Re-gera a mesh de um chunk já existente, passando o callback de vizinhos
    void remeshChunk(glm::ivec3 key);

    // Re-gera a mesh dos 4 vizinhos horizontais de um chunk
    void remeshNeighbors(glm::ivec3 key);
};