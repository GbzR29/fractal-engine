#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <optional>
#include "fractal_engine/world/Raycast.h"

namespace fractal_engine::renderer {

// ─────────────────────────────────────────────────────────────────────────────
// BlockOutline
// Renderiza um wireframe ao redor do bloco que o jogador está mirando.
//
// USO:
//   BlockOutline outline;
//   outline.init();
//
//   // no render loop:
//   auto hit = Raycast::raycast(...);
//   outline.render(hit, view, projection);
//
//   // ao encerrar:
//   outline.cleanup();
// ─────────────────────────────────────────────────────────────────────────────
class BlockOutline {
public:

    // ── Inicializa VAO/VBO com as 12 arestas do cubo unitário ─────────────
    void init() {
        // 8 vértices do cubo [0,1]^3
        // Expandidos levemente para evitar z-fighting com as faces do bloco
        const float E = 0.002f; // epsilon de expansão
        const float LO = 0.0f - E;
        const float HI = 1.0f + E;

        // 12 arestas = 24 vértices (2 por aresta com GL_LINES)
        float verts[] = {
            // arestas da face inferior (y = LO)
            LO,LO,LO,  HI,LO,LO,
            HI,LO,LO,  HI,LO,HI,
            HI,LO,HI,  LO,LO,HI,
            LO,LO,HI,  LO,LO,LO,

            // arestas da face superior (y = HI)
            LO,HI,LO,  HI,HI,LO,
            HI,HI,LO,  HI,HI,HI,
            HI,HI,HI,  LO,HI,HI,
            LO,HI,HI,  LO,HI,LO,

            // arestas verticais ligando inferior ↔ superior
            LO,LO,LO,  LO,HI,LO,
            HI,LO,LO,  HI,HI,LO,
            HI,LO,HI,  HI,HI,HI,
            LO,LO,HI,  LO,HI,HI,
        };

        glGenVertexArrays(1, &vao_);
        glGenBuffers(1, &vbo_);

        glBindVertexArray(vao_);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_);
        glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glBindVertexArray(0);

        compileShader();
        initialized_ = true;
    }

    // ── Renderiza o outline se houver hit ────────────────────────────────
    void render(
        const std::optional<fractal_engine::world::RaycastHit>& hit,
        const glm::mat4& view,
        const glm::mat4& projection)
    {
        if (!initialized_ || !hit.has_value()) return;

        glm::vec3 blockWorldPos = glm::vec3(hit->blockPos);

        // Translada o cubo unitário para a posição do bloco
        glm::mat4 model = glm::translate(glm::mat4(1.0f), blockWorldPos);

        glUseProgram(shader_);
        glUniformMatrix4fv(glGetUniformLocation(shader_, "model"),      1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(glGetUniformLocation(shader_, "view"),       1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shader_, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

        // Outline preto levemente transparente
        glUniform4f(glGetUniformLocation(shader_, "lineColor"), 0.0f, 0.0f, 0.0f, 0.6f);

        // Estado OpenGL para o outline
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // Desabilita escrita de profundidade, mas mantém teste
        // para o outline não aparecer através de paredes
        glDepthMask(GL_FALSE);

        // Offset poligonal evita z-fighting
        glEnable(GL_POLYGON_OFFSET_LINE);
        glPolygonOffset(-1.0f, -1.0f);

        glLineWidth(1.8f);

        glBindVertexArray(vao_);
        glDrawArrays(GL_LINES, 0, 24); // 12 arestas × 2 vértices
        glBindVertexArray(0);

        // Restaura estado
        glPolygonOffset(0.0f, 0.0f);
        glDisable(GL_POLYGON_OFFSET_LINE);
        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);
    }

    void cleanup() {
        if (vao_) glDeleteVertexArrays(1, &vao_);
        if (vbo_) glDeleteBuffers(1, &vbo_);
        if (shader_) glDeleteProgram(shader_);
    }

private:
    GLuint vao_     = 0;
    GLuint vbo_     = 0;
    GLuint shader_  = 0;
    bool   initialized_ = false;

    void compileShader() {
        // ── Vertex Shader ─────────────────────────────────────────────────
        const char* vert = R"glsl(
            #version 330 core
            layout(location = 0) in vec3 aPos;

            uniform mat4 model;
            uniform mat4 view;
            uniform mat4 projection;

            void main() {
                gl_Position = projection * view * model * vec4(aPos, 1.0);
            }
        )glsl";

        // ── Fragment Shader ───────────────────────────────────────────────
        const char* frag = R"glsl(
            #version 330 core
            uniform vec4 lineColor;
            out vec4 FragColor;

            void main() {
                FragColor = lineColor;
            }
        )glsl";

        auto compile = [](const char* src, GLenum type) -> GLuint {
            GLuint s = glCreateShader(type);
            glShaderSource(s, 1, &src, nullptr);
            glCompileShader(s);
            return s;
        };

        GLuint vs = compile(vert, GL_VERTEX_SHADER);
        GLuint fs = compile(frag, GL_FRAGMENT_SHADER);

        shader_ = glCreateProgram();
        glAttachShader(shader_, vs);
        glAttachShader(shader_, fs);
        glLinkProgram(shader_);

        glDeleteShader(vs);
        glDeleteShader(fs);
    }
};

} // namespace fractal_engine::renderer