#pragma once
#include <third_party/glad/glad.h>
#include <third_party/glm/glm.hpp>
#include "fractal_engine/graphics/Shader.h"
#include "fractal_engine/scene/Hotbar.h"

namespace fractal_engine::scene {

using fractal_engine::graphics::Shader;

// ─────────────────────────────────────────────────────────────────────────────
// HudRenderer — renderiza hotbar, highlight e stack count
//
// Usa dois shaders:
//   hudSlotShader : renderiza o fundo/borda dos slots (quads coloridos)
//   hudIconShader : amostra o texArray para mostrar o ícone do bloco
//
// Coordenadas: NDC [-1, 1], Y positivo para cima
// ─────────────────────────────────────────────────────────────────────────────
class HudRenderer {
public:
    HudRenderer() = default;
    ~HudRenderer();

    HudRenderer(const HudRenderer&)            = delete;
    HudRenderer& operator=(const HudRenderer&) = delete;

    // Inicializa VAOs e shaders
    void init(Shader& slotShader, Shader& iconShader);

    // Renderiza a hotbar completa
    // texArray: GL_TEXTURE_2D_ARRAY do BlockRegistry
    void render(const Hotbar& hotbar, GLuint texArray,
                int screenW, int screenH);

private:
    Shader* slotShader = nullptr;
    Shader* iconShader = nullptr;

    // ── Geometria de slots ────────────────────────────────────────────────
    GLuint slotVAO = 0, slotVBO = 0;
    GLuint iconVAO = 0, iconVBO = 0;
    GLuint textVAO = 0, textVBO = 0;

    // ── Layout da hotbar (em NDC) ──────────────────────────────────────────
    static constexpr int   SLOTS       = Hotbar::SLOTS;
    static constexpr float SLOT_SIZE   = 0.08f;   // tamanho de um slot em NDC
    static constexpr float SLOT_GAP    = 0.005f;  // espaço entre slots
    static constexpr float SLOT_Y      = -0.88f;  // centro Y da hotbar
    static constexpr float SLOT_BORDER = 0.005f;  // espessura da borda

    // ── Helpers ───────────────────────────────────────────────────────────
    float slotCenterX(int index, float aspect) const;

    void drawQuad(GLuint vao, GLuint vbo, Shader& shader,
                  float x, float y, float w, float h,
                  glm::vec4 color, float aspect);

    void drawIcon(int slotIndex, int texLayer,
                  float aspect);

    void drawText(const char* text, float ndcX, float ndcY,
                  float scale, glm::vec3 color, float aspect);

    void initBuffers();
};

} // namespace fractal_engine::scene