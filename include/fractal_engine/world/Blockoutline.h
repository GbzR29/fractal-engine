#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <optional>
#include "fractal_engine/world/Raycast.h"

namespace fractal_engine::renderer {

class BlockOutline {
public:

    void init() {
        // FIX: Expandir o outline levemente em escala (em vez de offset nos vértices)
        // garante que o wireframe abraça o bloco uniformemente em todas as faces.
        // Escala 1.004 centrada em 0.5,0.5,0.5 equivale a ±0.002 por lado.
        float verts[] = {
            // face inferior (y=0)
            0,0,0,  1,0,0,
            1,0,0,  1,0,1,
            1,0,1,  0,0,1,
            0,0,1,  0,0,0,
            // face superior (y=1)
            0,1,0,  1,1,0,
            1,1,0,  1,1,1,
            1,1,1,  0,1,1,
            0,1,1,  0,1,0,
            // verticais
            0,0,0,  0,1,0,
            1,0,0,  1,1,0,
            1,0,1,  1,1,1,
            0,0,1,  0,1,1,
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

    void render(
        const std::optional<fractal_engine::world::RaycastHit>& hit,
        const glm::mat4& view,
        const glm::mat4& projection)
    {
        if (!initialized_ || !hit.has_value()) return;

        // FIX: usa explicitamente blockPos (o bloco SÓLIDO atingido),
        // não adjacentPos (o ar onde o bloco seria colocado).
        glm::vec3 pos = glm::vec3(hit->blockPos);

        // Escala levemente maior que 1.0 centralizada no bloco para
        // evitar z-fighting sem depender de GL_POLYGON_OFFSET_LINE
        // (que não funciona de forma confiável em todos os drivers).
        const float S = 1.004f;
        const float O = (1.0f - S) * 0.5f; // offset para centralizar: -0.002

        glm::mat4 model = glm::translate(glm::mat4(1.0f), pos + glm::vec3(O));
        model = glm::scale(model, glm::vec3(S));

        glUseProgram(shader_);
        glUniformMatrix4fv(glGetUniformLocation(shader_, "model"),      1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(glGetUniformLocation(shader_, "view"),       1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shader_, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniform4f(glGetUniformLocation(shader_, "lineColor"), 0.0f, 0.0f, 0.0f, 0.8f);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDepthMask(GL_FALSE);
        glLineWidth(1.8f);

        glBindVertexArray(vao_);
        glDrawArrays(GL_LINES, 0, 24);
        glBindVertexArray(0);

        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);
    }

    void cleanup() {
        if (vao_) glDeleteVertexArrays(1, &vao_);
        if (vbo_) glDeleteBuffers(1, &vbo_);
        if (shader_) glDeleteProgram(shader_);
    }

private:
    GLuint vao_         = 0;
    GLuint vbo_         = 0;
    GLuint shader_      = 0;
    bool   initialized_ = false;

    void compileShader() {
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