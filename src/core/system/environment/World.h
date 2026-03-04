#pragma once

#include <map>
#include <glm/glm.hpp>
#include "Chunk.h"

// Estrutura para usar glm::ivec3 como chave no std::map
struct ChunkPosComparator {
    bool operator()(const glm::ivec3& a, const glm::ivec3& b) const {
        if (a.x != b.x) return a.x < b.x;
        if (a.y != b.y) return a.y < b.y;
        return a.z < b.z;
    }
};

class World {
public:
    World() = default;

    // Adiciona um chunk manualmente ou via geração
    void addChunk(glm::ivec3 pos, Shader& shader);

    // Verifica se existe um bloco sólido em coordenadas globais
    bool isBlockSolid(float wx, float wy, float wz);

    // Renderiza todos os chunks carregados
    void render(Shader& shader);

    // Mapa de chunks usando a posição global do chunk como chave
    std::map<glm::ivec3, Chunk, ChunkPosComparator> chunks;

private:
    // Utilitário para converter coord. global em coord. de Chunk
    glm::ivec3 getChunkCoords(int wx, int wy, int wz);
};