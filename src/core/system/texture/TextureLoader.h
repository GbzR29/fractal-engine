#pragma once
#include <glad/glad.h>
#include <string>
#include <unordered_map>

class TextureLoader {
public:
    // Interface original mantida — compatível com o Chunk.cpp existente
    TextureLoader(GLuint texID, const char* path);

    // Versão estática com cache: retorna um texID gerenciado internamente.
    // Se o mesmo path já foi carregado, devolve o ID existente sem reprocessar.
    static GLuint load(const char* path);

    // Libera todas as texturas do cache (chamar ao encerrar o programa)
    static void clearCache();

private:
    static void loadInto(GLuint texID, const char* path);

    // path -> OpenGL texture ID
    static std::unordered_map<std::string, GLuint> s_cache;
};