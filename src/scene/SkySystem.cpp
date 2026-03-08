#include "fractal_engine/scene/SkySystem.h"
#include <cmath>
#include <algorithm>

namespace fractal_engine::scene {

// ─────────────────────────────────────────────────────────────────────────────
// Vértices do cubo unitário invertido (face interna visível)
// 36 vértices (6 faces × 2 triângulos × 3 vértices)
// ─────────────────────────────────────────────────────────────────────────────
static const float kSkyboxVertices[] = {
    // +X
    1,-1,-1,  1,-1, 1,  1, 1, 1,  1, 1, 1,  1, 1,-1,  1,-1,-1,
    // -X
   -1,-1, 1, -1,-1,-1, -1, 1,-1, -1, 1,-1, -1, 1, 1, -1,-1, 1,
    // +Y (teto)
   -1, 1,-1,  1, 1,-1,  1, 1, 1,  1, 1, 1, -1, 1, 1, -1, 1,-1,
    // -Y (chão — não aparece mas mantém o cubo fechado)
   -1,-1, 1,  1,-1, 1,  1,-1,-1,  1,-1,-1, -1,-1,-1, -1,-1, 1,
    // +Z
   -1,-1, 1, -1, 1, 1,  1, 1, 1,  1, 1, 1,  1,-1, 1, -1,-1, 1,
    // -Z
    1,-1,-1,  1, 1,-1, -1, 1,-1, -1, 1,-1, -1,-1,-1,  1,-1,-1,
};

// ── Destrutor ─────────────────────────────────────────────────────────────────
SkySystem::~SkySystem() {
    if (skyVAO) glDeleteVertexArrays(1, &skyVAO);
    if (skyVBO) glDeleteBuffers(1, &skyVBO);
}

// ── Init ──────────────────────────────────────────────────────────────────────
void SkySystem::init(Shader& shader, const Config& cfg) {
    skyShader = &shader;
    config    = cfg;
    initSkyboxMesh();
    recalculate();
}

// ── Mesh do cubo ──────────────────────────────────────────────────────────────
void SkySystem::initSkyboxMesh() {
    glGenVertexArrays(1, &skyVAO);
    glGenBuffers(1, &skyVBO);

    glBindVertexArray(skyVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(kSkyboxVertices),
                 kSkyboxVertices, GL_STATIC_DRAW);

    // location 0: vec3 aPos
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                          3 * sizeof(float), (void*)0);

    glBindVertexArray(0);
}

// ── Update ────────────────────────────────────────────────────────────────────
void SkySystem::update(float dt) {
    if (config.paused) return;
    timeOfDay += dt / config.dayDurationSeconds;
    if (timeOfDay > 1.0f) timeOfDay -= 1.0f;
    recalculate();
}

// ── Render ────────────────────────────────────────────────────────────────────
void SkySystem::render(const glm::mat4& view, const glm::mat4& projection) {
    if (!skyShader || !skyVAO) return;

    // Salva estado
    GLint prevDepthFunc;
    glGetIntegerv(GL_DEPTH_FUNC, &prevDepthFunc);

    // Skybox passa no depth test quando z = w (profundidade máxima)
    glDepthFunc(GL_LEQUAL);
    // Não escreve no depth buffer — geometria do mundo vai sobrepor
    glDepthMask(GL_FALSE);

    skyShader->use();
    skyShader->setMat4 ("view",           view);
    skyShader->setMat4 ("projection",     projection);
    skyShader->setFloat("uTimeOfDay",     timeOfDay);
    skyShader->setFloat("uSunIntensity",  sunIntensity);
    skyShader->setVec3 ("uSunColor",      sunColor);

    glBindVertexArray(skyVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);

    // Restaura estado
    glDepthMask(GL_TRUE);
    glDepthFunc(prevDepthFunc);
}

// ── applyToShader ─────────────────────────────────────────────────────────────
void SkySystem::applyToShader(Shader& shader) const {
    shader.use();
    shader.setFloat("sunIntensity", sunIntensity);
    shader.setVec3 ("sunColor",     sunColor);
    shader.setFloat("ambientMin",   config.ambientMin);
}

// ── setTimeOfDay ──────────────────────────────────────────────────────────────
void SkySystem::setTimeOfDay(float t) {
    timeOfDay = t - std::floor(t);
    recalculate();
}

// ─────────────────────────────────────────────────────────────────────────────
// recalculate
// Ciclo: 0.0=meia-noite, 0.25=amanhecer, 0.5=meio-dia, 0.75=pôr-do-sol
// ─────────────────────────────────────────────────────────────────────────────
void SkySystem::recalculate() {
    constexpr float PI = 3.14159265f;

    // ── Sun intensity ─────────────────────────────────────────────────────
    float raw = std::sin(timeOfDay * 2.0f * PI - PI * 0.5f);
    sunIntensity = std::max(0.0f, (raw + 1.0f) * 0.5f);

    // ── Sun color ─────────────────────────────────────────────────────────
    const glm::vec3 colorNoon   { 1.00f, 0.98f, 0.85f };
    const glm::vec3 colorHorizon{ 1.00f, 0.60f, 0.20f };
    float horizonBlend = smoothstep(0.0f, 0.3f, sunIntensity);
    sunColor = glm::mix(colorHorizon, colorNoon, horizonBlend);

    // ── Clear color ───────────────────────────────────────────────────────
    // (usado como fallback antes do skybox ser renderizado)
    const glm::vec3 skyDay   { 0.38f, 0.76f, 0.90f };
    const glm::vec3 skyDusk  { 0.70f, 0.35f, 0.10f };
    const glm::vec3 skyNight { 0.02f, 0.02f, 0.08f };

    if (sunIntensity > 0.3f) {
        float t  = smoothstep(0.3f, 0.7f, sunIntensity);
        clearColor = glm::mix(skyDusk, skyDay, t);
    } else {
        float t  = smoothstep(0.0f, 0.3f, sunIntensity);
        clearColor = glm::mix(skyNight, skyDusk, t);
    }
}

float SkySystem::smoothstep(float edge0, float edge1, float x) {
    float t = std::max(0.0f, std::min(1.0f, (x - edge0) / (edge1 - edge0)));
    return t * t * (3.0f - 2.0f * t);
}

} // namespace fractal_engine::scene