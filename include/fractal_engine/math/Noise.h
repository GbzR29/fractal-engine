#pragma once
#include <cstdint>
#include <array>

namespace fractal_engine::math {

// ─────────────────────────────────────────────────────────────────────────────
// Noise — Perlin Noise 2D e 3D com suporte a fractal (fBm)
//
// Implementação baseada em "Improved Noise" de Ken Perlin (2002).
// A tabela de permutação é embaralhada pelo seed no construtor, garantindo
// mundos determinísticos por seed.
//
// fractalNoise / fractalNoise3D retornam valores em [0.0, 1.0] (normalizado)
// noise2D / noise3D retornam valores em [-1.0, 1.0] (raw Perlin)
// ─────────────────────────────────────────────────────────────────────────────
class Noise {
public:
    explicit Noise(uint32_t seed = 0);

    // ── Raw Perlin ─────────────────────────────────────────────────────────
    float noise2D(float x, float y)          const;
    float noise3D(float x, float y, float z) const;

    // ── Fractal Brownian Motion ────────────────────────────────────────────
    // persistence: amplitude de cada oitava (tipicamente 0.4 ~ 0.6)
    // octaves: número de camadas de detalhe
    // retorna valor normalizado em [0, 1]
    float fractalNoise(float x, float z,
                       float persistence = 0.5f,
                       int   octaves     = 4)    const;

    float fractalNoise3D(float x, float y, float z,
                         float persistence = 0.5f,
                         int   octaves     = 4)   const;

private:
    std::array<int, 512> perm;  // tabela de permutação duplicada

    static float fade(float t);
    static float lerp(float a, float b, float t);
    static float grad2(int hash, float x, float y);
    static float grad3(int hash, float x, float y, float z);
};

} // namespace fractal_engine::math