#pragma once
#include <map>
#include <third_party/glm/glm.hpp>
#include "fractal_engine/world/Chunk.h"
#include "fractal_engine/graphics/Shader.h"

namespace fractal_engine::world {

using fractal_engine::graphics::Shader;

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

    /**
     * @brief Encontra a altura do primeiro bloco sólido a partir de uma altura máxima
     * 
     * @param worldX Coordenada X
     * @param worldZ Coordenada Z
     * @param maxHeight Altura máxima para procurar (padrão: 100)
     * 
     * @return A altura do primeiro bloco sólido encontrado, ou -1 se nenhum for encontrado
     */
    float getTerrainHeightAt(float worldX, float worldZ, float maxHeight = 100.0f) const;

    void render(Shader& shader);

    // ─────────────────────────────────────────────
    // SISTEMA DE QUEBRA/CONSTRUÇÃO DE BLOCOS
    // ─────────────────────────────────────────────

    // Quebra um bloco na posição especificada
    // Retorna true se conseguiu quebrar
    bool breakBlock(glm::ivec3 pos);

    // Coloca um bloco na posição especificada (se estiver vazio)
    // Retorna true se conseguiu colocar
    bool placeBlock(glm::ivec3 pos, BlockType blockType = BLOCK_STONE);

private:
    // Re-gera a mesh de um chunk já existente, passando o callback de vizinhos
    void remeshChunk(glm::ivec3 key);

    // Re-gera a mesh dos 4 vizinhos horizontais de um chunk
    void remeshNeighbors(glm::ivec3 key);
};

} // namespace fractal_engine::world