#pragma once
#include <third_party/glad/glad.h>
#include <third_party/glm/glm.hpp>
#include "fractal_engine/graphics/Shader.h"

namespace fractal_engine::scene {

using fractal_engine::graphics::Shader;

// ─────────────────────────────────────────────────────────────────────────────
// SkySystem — controla o ciclo de dia e noite + skybox procedural
//
// O skybox é um cubo invertido renderizado com depth trick (z/w = 1.0),
// sempre desenhado ANTES da geometria do mundo.
//
// USO BÁSICO:
//   // Na inicialização:
//   sky.init(skyShader);                 // passa o shader compilado
//
//   // A cada frame:
//   sky.update(dt);
//   sky.render(view, projection);        // desenha o skybox
//   sky.applyToShader(worldShader);      // ilumina o mundo
//   glClearColor(sky.getClearColor()...);
//
// UNIFORMS esperados em sky.vert / sky.frag:
//   uTimeOfDay    : float
//   uSunIntensity : float
//   uSunColor     : vec3
//   view          : mat4
//   projection    : mat4
// ─────────────────────────────────────────────────────────────────────────────

struct SkyConfig {
    float dayDurationSeconds = 600.0f; // duração de um dia completo (10 min)
    float ambientMin         = 0.08f;  // luz mínima à noite
    bool  paused             = false;  // congela o tempo
};

class SkySystem {
public:
    using Config = SkyConfig;

    SkySystem() = default;
    ~SkySystem();

    // Inicializa o sistema e cria o VAO do cubo do skybox
    // skyShader deve apontar para sky.vert / sky.frag
    void init(Shader& skyShader, const Config& cfg = Config{});

    // Avança o tempo — chame todo frame com dt em segundos
    void update(float dt);

    // Renderiza o skybox — chame ANTES de desenhar o mundo
    // Desabilita depth write internamente
    void render(const glm::mat4& view, const glm::mat4& projection);

    // Passa sunIntensity, sunColor, ambientMin para o shader do mundo
    void applyToShader(Shader& shader) const;

    // Cor do céu para glClearColor
    glm::vec3 getClearColor() const { return clearColor; }

    // ── Controle manual ───────────────────────────────────────────────────
    // timeOfDay: 0.0=meia-noite, 0.25=amanhecer, 0.5=meio-dia, 0.75=pôr-do-sol
    void  setTimeOfDay(float t);
    float getTimeOfDay()    const { return timeOfDay; }
    float getSunIntensity() const { return sunIntensity; }
    void  setConfig(const Config& cfg) { config = cfg; }

private:
    Config    config;
    Shader*   skyShader    = nullptr;
    GLuint    skyVAO       = 0;
    GLuint    skyVBO       = 0;

    float     timeOfDay    = 0.5f;
    float     sunIntensity = 1.0f;
    glm::vec3 sunColor     { 1.0f, 0.98f, 0.85f };
    glm::vec3 clearColor   { 0.38f, 0.76f, 0.9f };

    void recalculate();
    void initSkyboxMesh();

    static float smoothstep(float edge0, float edge1, float x);
};

} // namespace fractal_engine::scene