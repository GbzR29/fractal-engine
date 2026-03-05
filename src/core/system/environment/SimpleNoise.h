#pragma once
#include <cmath>

class SimpleNoise {
public:
    unsigned int seed;

    SimpleNoise(unsigned int s = 1337) : seed(s) {}

    // Hash determinístico 2D com seed embutido
    float random2D(int x, int z) const {
        unsigned int n = (unsigned int)(x * 1619 + z * 31337 + seed * 1013904223);
        n = (n ^ (n >> 13)) * 1274126177u;
        n = (n ^ (n >> 16));
        return (float)(n & 0x7fffffff) / (float)0x7fffffff; // [0, 1]
    }

    float lerp(float a, float b, float t) const {
        return a + t * (b - a);
    }

    // Smoothstep cúbico (evita artefatos de banda)
    float smooth(float t) const {
        return t * t * (3.0f - 2.0f * t);
    }

    float sample2D(float x, float z) const {
        int x0 = (int)std::floor(x);
        int x1 = x0 + 1;
        int z0 = (int)std::floor(z);
        int z1 = z0 + 1;

        float tx = smooth(x - x0);
        float tz = smooth(z - z0);

        float v00 = random2D(x0, z0);
        float v10 = random2D(x1, z0);
        float v01 = random2D(x0, z1);
        float v11 = random2D(x1, z1);

        return lerp(lerp(v00, v10, tx), lerp(v01, v11, tx), tz);
    }

    // fractalNoise retorna valor em [0, 1]
    // scale: tamanho do bioma (quanto maior, mais suave)
    // octaves: detalhamento (4~6 é bom)
    float fractalNoise(float x, float z, float scale = 80.0f, int octaves = 5) const {
        float value     = 0.0f;
        float amplitude = 1.0f;
        float frequency = 1.0f / scale;
        float maxVal    = 0.0f;

        for (int i = 0; i < octaves; i++) {
            value   += sample2D(x * frequency, z * frequency) * amplitude;
            maxVal  += amplitude;
            amplitude  *= 0.5f;
            frequency  *= 2.0f;
        }

        return value / maxVal; // normalizado [0, 1]
    }
};