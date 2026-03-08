#include "fractal_engine/scene/HudRenderer.h"
#include "fractal_engine/world/BlockRegistry.h"

// stb_easy_font — header-only, incluir aqui para não poluir outros TUs
#define STB_EASY_FONT_IMPLEMENTATION
#include <third_party/stb_image/stb_easy_font.h>

#include <cstring>
#include <string>
#include <cstdio>

namespace fractal_engine::scene {

using fractal_engine::world::BlockRegistry;
using fractal_engine::world::BlockType;

// ─────────────────────────────────────────────────────────────────────────────
// Destrutor
// ─────────────────────────────────────────────────────────────────────────────
HudRenderer::~HudRenderer() {
    if (slotVAO) { glDeleteVertexArrays(1, &slotVAO); glDeleteBuffers(1, &slotVBO); }
    if (iconVAO) { glDeleteVertexArrays(1, &iconVAO); glDeleteBuffers(1, &iconVBO); }
    if (textVAO) { glDeleteVertexArrays(1, &textVAO); glDeleteBuffers(1, &textVBO); }
}

// ─────────────────────────────────────────────────────────────────────────────
// init
// ─────────────────────────────────────────────────────────────────────────────
void HudRenderer::init(Shader& slotSh, Shader& iconSh) {
    slotShader = &slotSh;
    iconShader = &iconSh;
    initBuffers();
}

void HudRenderer::initBuffers() {
    // Slot / borda / texto — quad genérico (6 vértices, 2 floats cada)
    glGenVertexArrays(1, &slotVAO); glGenBuffers(1, &slotVBO);
    glBindVertexArray(slotVAO);
    glBindBuffer(GL_ARRAY_BUFFER, slotVBO);
    glBufferData(GL_ARRAY_BUFFER, 6 * 2 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    // Ícone — quad com UV (6 vértices, 4 floats: xy + uv)
    glGenVertexArrays(1, &iconVAO); glGenBuffers(1, &iconVBO);
    glBindVertexArray(iconVAO);
    glBindBuffer(GL_ARRAY_BUFFER, iconVBO);
    glBufferData(GL_ARRAY_BUFFER, 6 * 4 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);

    // Texto — buffer maior para stb_easy_font (até 2000 quads)
    glGenVertexArrays(1, &textVAO); glGenBuffers(1, &textVBO);
    glBindVertexArray(textVAO);
    glBindBuffer(GL_ARRAY_BUFFER, textVBO);
    glBufferData(GL_ARRAY_BUFFER, 2000 * 4 * 4 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
}

// ─────────────────────────────────────────────────────────────────────────────
// slotCenterX — centro X do slot i em NDC
// ─────────────────────────────────────────────────────────────────────────────
float HudRenderer::slotCenterX(int index, float aspect) const {
    // Largura total da hotbar
    float totalW = SLOTS * SLOT_SIZE / aspect + (SLOTS - 1) * SLOT_GAP / aspect;
    float startX = -totalW * 0.5f;
    return startX + (index * (SLOT_SIZE / aspect + SLOT_GAP / aspect)) + SLOT_SIZE * 0.5f / aspect;
}

// ─────────────────────────────────────────────────────────────────────────────
// drawQuad — desenha um quad colorido usando slotShader
// x, y = centro do quad em NDC
// ─────────────────────────────────────────────────────────────────────────────
void HudRenderer::drawQuad(GLuint vao, GLuint vbo, Shader& shader,
                            float x, float y, float w, float h,
                            glm::vec4 color, float /*aspect*/)
{
    float hw = w * 0.5f, hh = h * 0.5f;
    float verts[12] = {
        x-hw, y-hh,   x+hw, y-hh,   x+hw, y+hh,
        x+hw, y+hh,   x-hw, y+hh,   x-hw, y-hh,
    };

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(verts), verts);

    shader.use();
    shader.setVec3("color", glm::vec3(color.r, color.g, color.b));
    shader.setFloat("alpha", color.a);

    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

// ─────────────────────────────────────────────────────────────────────────────
// drawIcon — amostra face top do bloco do texArray como ícone 2D
// ─────────────────────────────────────────────────────────────────────────────
void HudRenderer::drawIcon(int slotIndex, int texLayer, float aspect) {
    float cx = slotCenterX(slotIndex, aspect);
    float cy = SLOT_Y;
    float hw = (SLOT_SIZE - SLOT_BORDER * 2.0f) * 0.5f / aspect;
    float hh = (SLOT_SIZE - SLOT_BORDER * 2.0f) * 0.5f;

    // xy + uv por vértice
    float verts[24] = {
        cx-hw, cy-hh,  0.0f, 1.0f,
        cx+hw, cy-hh,  1.0f, 1.0f,
        cx+hw, cy+hh,  1.0f, 0.0f,
        cx+hw, cy+hh,  1.0f, 0.0f,
        cx-hw, cy+hh,  0.0f, 0.0f,
        cx-hw, cy-hh,  0.0f, 1.0f,
    };

    glBindVertexArray(iconVAO);
    glBindBuffer(GL_ARRAY_BUFFER, iconVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(verts), verts);

    iconShader->use();
    iconShader->setInt("texArray", 0);
    iconShader->setInt("texLayer", texLayer);

    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

// ─────────────────────────────────────────────────────────────────────────────
// drawText — renderiza string com stb_easy_font em NDC
// ─────────────────────────────────────────────────────────────────────────────
void HudRenderer::drawText(const char* text, float ndcX, float ndcY,
                            float scale, glm::vec3 color, float aspect)
{
    // stb_easy_font trabalha em pixels — usamos escala própria em NDC
    static float buf[2000 * 4 * 4]; // buffer de quads (x,y,z,w por vértice)
    int numQuads = stb_easy_font_print(0, 0, (char*)text, nullptr,
                                       buf, sizeof(buf));

    // stb retorna quads (4 verts cada) — precisa converter para triângulos
    // e escalar de pixels para NDC
    std::vector<float> verts;
    verts.reserve(numQuads * 6 * 2);

    for (int q = 0; q < numQuads; q++) {
        float* quad = buf + q * 4 * 4; // 4 vértices, 4 floats (x,y,z,w)
        // Índices do quad: 0,1,2 e 2,3,0
        int idx[6] = {0, 1, 2, 2, 3, 0};
        for (int i : idx) {
            float px = quad[i * 4 + 0];
            float py = quad[i * 4 + 1];
            // Escala de pixel para NDC
            verts.push_back(ndcX + px * scale / aspect);
            verts.push_back(ndcY - py * scale); // Y invertido
        }
    }

    glBindVertexArray(textVAO);
    glBindBuffer(GL_ARRAY_BUFFER, textVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0,
                    verts.size() * sizeof(float), verts.data());

    slotShader->use();
    slotShader->setVec3("color", color);
    slotShader->setFloat("alpha", 1.0f);

    glDrawArrays(GL_TRIANGLES, 0, (GLsizei)verts.size() / 2);
    glBindVertexArray(0);
}

// ─────────────────────────────────────────────────────────────────────────────
// render — ponto de entrada principal
// ─────────────────────────────────────────────────────────────────────────────
void HudRenderer::render(const Hotbar& hotbar, GLuint texArray,
                          int screenW, int screenH)
{
    float aspect = (float)screenW / (float)screenH;

    // Desliga depth test para UI
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // ── 1. Fundo de cada slot ─────────────────────────────────────────────
    for (int i = 0; i < SLOTS; i++) {
        float cx = slotCenterX(i, aspect);
        float sw = SLOT_SIZE / aspect;
        float sh = SLOT_SIZE;

        // Borda (slot selecionado = dourada, outros = cinza escuro)
        glm::vec4 borderColor = (i == hotbar.getSelectedIndex())
            ? glm::vec4(1.0f, 0.85f, 0.1f, 1.0f)   // dourado
            : glm::vec4(0.15f, 0.15f, 0.15f, 0.85f); // cinza escuro

        drawQuad(slotVAO, slotVBO, *slotShader,
                 cx, SLOT_Y, sw, sh, borderColor, aspect);

        // Interior (fundo mais claro dentro da borda)
        float bw = SLOT_BORDER / aspect;
        float bh = SLOT_BORDER;
        glm::vec4 bgColor = (i == hotbar.getSelectedIndex())
            ? glm::vec4(0.35f, 0.30f, 0.10f, 0.90f)
            : glm::vec4(0.20f, 0.20f, 0.20f, 0.80f);

        drawQuad(slotVAO, slotVBO, *slotShader,
                 cx, SLOT_Y,
                 sw - bw * 2.0f, sh - bh * 2.0f,
                 bgColor, aspect);
    }

    // ── 2. Ícones dos blocos ──────────────────────────────────────────────
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, texArray);

    for (int i = 0; i < SLOTS; i++) {
        const SlotStack& slot = hotbar.getSlot(i);
        if (slot.isEmpty()) continue;

        const auto& def = BlockRegistry::get(slot.type);
        drawIcon(i, def.topLayer, aspect);
    }

    // ── 3. Stack count (canto inferior direito de cada slot) ──────────────
    for (int i = 0; i < SLOTS; i++) {
        const SlotStack& slot = hotbar.getSlot(i);
        if (slot.isEmpty() || slot.isInfinite()) continue;

        char buf[8];
        std::snprintf(buf, sizeof(buf), "%d", slot.count);

        float cx = slotCenterX(i, aspect);
        // Posição: canto inferior direito do slot
        float tx = cx + (SLOT_SIZE * 0.5f - SLOT_BORDER) / aspect * 0.6f;
        float ty = SLOT_Y - SLOT_SIZE * 0.5f + SLOT_BORDER * 2.0f;

        // Sombra
        drawText(buf, tx + 0.001f, ty - 0.001f, 0.0008f,
                 glm::vec3(0.0f), aspect);
        // Texto
        drawText(buf, tx, ty, 0.0008f,
                 glm::vec3(1.0f, 1.0f, 0.3f), aspect);
    }

    // ── 4. Nome do bloco selecionado (acima da hotbar) ────────────────────
    {
        const std::string& name = hotbar.getSelectedName();
        if (!name.empty() && hotbar.getSelectedBlock() != BlockType::BLOCK_AIR) {
            float textY = SLOT_Y + SLOT_SIZE * 0.5f + 0.03f;
            // Centraliza aproximadamente
            float textX = -(float)name.size() * 0.0004f;

            // Sombra
            drawText(name.c_str(), textX + 0.001f, textY - 0.001f,
                     0.001f, glm::vec3(0.0f), aspect);
            // Texto branco
            drawText(name.c_str(), textX, textY,
                     0.001f, glm::vec3(1.0f), aspect);
        }
    }

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
}

} // namespace fractal_engine::scene