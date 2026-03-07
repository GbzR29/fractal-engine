#include "fractal_engine/math/Noise.h"
#include <numeric>
#include <algorithm>
#include <random>
#include <cmath>

namespace fractal_engine::math {

// ─────────────────────────────────────────────────────────────────────────────
// Construtor — embaralha a tabela de permutação com o seed
// ─────────────────────────────────────────────────────────────────────────────
Noise::Noise(uint32_t seed) {
    // Tabela base [0..255]
    std::array<int, 256> base;
    std::iota(base.begin(), base.end(), 0);

    // Shuffle via Mersenne Twister com seed fornecido
    std::mt19937 rng(seed);
    std::shuffle(base.begin(), base.end(), rng);

    // Duplica para evitar mod no lookup
    for (int i = 0; i < 256; i++) {
        perm[i]       = base[i];
        perm[i + 256] = base[i];
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Funções auxiliares inline
// ─────────────────────────────────────────────────────────────────────────────
float Noise::fade(float t) {
    // Curva suave de Ken Perlin: 6t^5 - 15t^4 + 10t^3
    return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f);
}

float Noise::lerp(float a, float b, float t) {
    return a + t * (b - a);
}

// Gradiente 2D — 8 vetores unitários principais
float Noise::grad2(int hash, float x, float y) {
    switch (hash & 7) {
        case 0: return  x + y;
        case 1: return -x + y;
        case 2: return  x - y;
        case 3: return -x - y;
        case 4: return  x;
        case 5: return -x;
        case 6: return  y;
        case 7: return -y;
    }
    return 0.0f; // nunca chega aqui
}

// Gradiente 3D — 12 vetores de arestas do cubo unitário
// FIX: esta tabela é a versão "improved" do Perlin (2002).
// A versão anterior usava apenas 8 gradientes → bias visível nas cavernas.
float Noise::grad3(int hash, float x, float y, float z) {
    switch (hash & 15) {
        case  0: return  x + y;
        case  1: return -x + y;
        case  2: return  x - y;
        case  3: return -x - y;
        case  4: return  x + z;
        case  5: return -x + z;
        case  6: return  x - z;
        case  7: return -x - z;
        case  8: return  y + z;
        case  9: return -y + z;
        case 10: return  y - z;
        case 11: return -y - z;
        case 12: return  x + y;  // repetidos para distribuição uniforme
        case 13: return -x + y;
        case 14: return -y + z;
        case 15: return -y - z;
    }
    return 0.0f;
}

// ─────────────────────────────────────────────────────────────────────────────
// Perlin 2D — retorna [-1, 1]
// ─────────────────────────────────────────────────────────────────────────────
float Noise::noise2D(float x, float y) const {
    int xi = (int)std::floor(x) & 255;
    int yi = (int)std::floor(y) & 255;

    float xf = x - std::floor(x);
    float yf = y - std::floor(y);

    float u = fade(xf);
    float v = fade(yf);

    int aa = perm[perm[xi    ] + yi    ];
    int ab = perm[perm[xi    ] + yi + 1];
    int ba = perm[perm[xi + 1] + yi    ];
    int bb = perm[perm[xi + 1] + yi + 1];

    float x1 = lerp(grad2(aa, xf,       yf      ),
                    grad2(ba, xf - 1.0f, yf      ), u);
    float x2 = lerp(grad2(ab, xf,       yf - 1.0f),
                    grad2(bb, xf - 1.0f, yf - 1.0f), u);

    return lerp(x1, x2, v);
}

// ─────────────────────────────────────────────────────────────────────────────
// Perlin 3D — retorna [-1, 1]
//
// FIX: A implementação anterior fazia floor() sem aplicar & 255,
// causando índices negativos na tabela perm[] quando x/y/z < 0.
// Índices negativos em std::array resultam em UB → ruído completamente
// incorreto (ou crash) para coordenadas negativas do mundo.
// ─────────────────────────────────────────────────────────────────────────────
float Noise::noise3D(float x, float y, float z) const {
    // FIX: usa & 255 após floor para garantir índice sempre positivo
    int xi = (int)std::floor(x) & 255;
    int yi = (int)std::floor(y) & 255;
    int zi = (int)std::floor(z) & 255;

    float xf = x - std::floor(x);
    float yf = y - std::floor(y);
    float zf = z - std::floor(z);

    float u = fade(xf);
    float v = fade(yf);
    float w = fade(zf);

    // Lookup da tabela de permutação — os 8 cantos do cubo
    int aaa = perm[perm[perm[xi    ] + yi    ] + zi    ];
    int aab = perm[perm[perm[xi    ] + yi    ] + zi + 1];
    int aba = perm[perm[perm[xi    ] + yi + 1] + zi    ];
    int abb = perm[perm[perm[xi    ] + yi + 1] + zi + 1];
    int baa = perm[perm[perm[xi + 1] + yi    ] + zi    ];
    int bab = perm[perm[perm[xi + 1] + yi    ] + zi + 1];
    int bba = perm[perm[perm[xi + 1] + yi + 1] + zi    ];
    int bbb = perm[perm[perm[xi + 1] + yi + 1] + zi + 1];

    // Interpola os 8 cantos
    float x1 = lerp(grad3(aaa, xf,       yf,       zf      ),
                    grad3(baa, xf - 1.0f, yf,       zf      ), u);
    float x2 = lerp(grad3(aba, xf,       yf - 1.0f, zf      ),
                    grad3(bba, xf - 1.0f, yf - 1.0f, zf      ), u);
    float y1 = lerp(x1, x2, v);

    float x3 = lerp(grad3(aab, xf,       yf,       zf - 1.0f),
                    grad3(bab, xf - 1.0f, yf,       zf - 1.0f), u);
    float x4 = lerp(grad3(abb, xf,       yf - 1.0f, zf - 1.0f),
                    grad3(bbb, xf - 1.0f, yf - 1.0f, zf - 1.0f), u);
    float y2 = lerp(x3, x4, v);

    return lerp(y1, y2, w);
}

// ─────────────────────────────────────────────────────────────────────────────
// Fractal Brownian Motion 2D — retorna [0, 1]
// persistence: quanto a amplitude diminui por oitava (0.5 = metade)
// ─────────────────────────────────────────────────────────────────────────────
float Noise::fractalNoise(float x, float z, float persistence, int octaves) const {
    float value     = 0.0f;
    float amplitude = 1.0f;
    float frequency = 1.0f;
    float maxValue  = 0.0f;

    for (int i = 0; i < octaves; i++) {
        // noise2D retorna [-1, 1], converte para [0, 1]
        float sample = (noise2D(x * frequency, z * frequency) + 1.0f) * 0.5f;
        value    += sample * amplitude;
        maxValue += amplitude;

        amplitude *= persistence;
        frequency *= 2.0f;  // lacunarity = 2 (dobra frequência por oitava)
    }

    return value / maxValue;  // normaliza para [0, 1]
}

// ─────────────────────────────────────────────────────────────────────────────
// Fractal Brownian Motion 3D — retorna [0, 1]
// ─────────────────────────────────────────────────────────────────────────────
float Noise::fractalNoise3D(float x, float y, float z, float persistence, int octaves) const {
    float value     = 0.0f;
    float amplitude = 1.0f;
    float frequency = 1.0f;
    float maxValue  = 0.0f;

    for (int i = 0; i < octaves; i++) {
        // noise3D retorna [-1, 1], converte para [0, 1]
        float sample = (noise3D(x * frequency, y * frequency, z * frequency) + 1.0f) * 0.5f;
        value    += sample * amplitude;
        maxValue += amplitude;

        amplitude *= persistence;
        frequency *= 2.0f;
    }

    return value / maxValue;  // normaliza para [0, 1]
}

} // namespace fractal_engine::math