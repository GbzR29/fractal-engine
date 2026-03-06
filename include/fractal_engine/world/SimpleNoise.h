#pragma once
#include <cmath>

class SimpleNoise {
public:
    unsigned int seed;

    SimpleNoise(unsigned int s = 1337) : seed(s) {}

    // ─────────────────────────────────────────────
    // PERLIN 2D (código original)
    // ─────────────────────────────────────────────

    // Hash determinístico 2D com seed embutido
    float random2D(int x, int z) const {
        unsigned int n = (unsigned int)(x * 1619 + z * 31337 + seed * 1013904223);
        n = (n ^ (n >> 13)) * 1274126177u;
        n = (n ^ (n >> 16));
        return (float)(n & 0x7fffffff) / (float)0x7fffffff; // [0, 1]
    }

    // ─────────────────────────────────────────────
    // PERLIN 3D (novo!)
    // ─────────────────────────────────────────────

    // Hash determinístico 3D com seed embutido
    float random3D(int x, int y, int z) const {
        unsigned int n = (unsigned int)(
            x * 1619 + 
            y * 571 + 
            z * 31337 + 
            seed * 1013904223
        );
        n = (n ^ (n >> 13)) * 1274126177u;
        n = (n ^ (n >> 16));
        return (float)(n & 0x7fffffff) / (float)0x7fffffff; // [0, 1]
    }

    // ─────────────────────────────────────────────
    // Interpolação e smoothing
    // ─────────────────────────────────────────────

    float lerp(float a, float b, float t) const {
        return a + t * (b - a);
    }

    // Smoothstep cúbico (evita artefatos de banda)
    float smooth(float t) const {
        return t * t * (3.0f - 2.0f * t);
    }

    // ─────────────────────────────────────────────
    // Amostragem 2D (original)
    // ─────────────────────────────────────────────

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

    // ─────────────────────────────────────────────
    // Amostragem 3D (Perlin Noise bidimensional interpolado em 3D)
    // ─────────────────────────────────────────────

    float sample3D(float x, float y, float z) const {
        int x0 = (int)std::floor(x);
        int x1 = x0 + 1;
        int y0 = (int)std::floor(y);
        int y1 = y0 + 1;
        int z0 = (int)std::floor(z);
        int z1 = z0 + 1;

        float tx = smooth(x - x0);
        float ty = smooth(y - y0);
        float tz = smooth(z - z0);

        // 8 vértices do cubo
        float v000 = random3D(x0, y0, z0);
        float v100 = random3D(x1, y0, z0);
        float v010 = random3D(x0, y1, z0);
        float v110 = random3D(x1, y1, z0);
        float v001 = random3D(x0, y0, z1);
        float v101 = random3D(x1, y0, z1);
        float v011 = random3D(x0, y1, z1);
        float v111 = random3D(x1, y1, z1);

        // Interpolação trilinear
        // Interpolar no eixo X
        float v00 = lerp(v000, v100, tx);
        float v10 = lerp(v010, v110, tx);
        float v01 = lerp(v001, v101, tx);
        float v11 = lerp(v011, v111, tx);
        
        // Interpolar no eixo Y
        float v0 = lerp(v00, v10, ty);
        float v1 = lerp(v01, v11, ty);
        
        // Interpolar no eixo Z
        return lerp(v0, v1, tz);
    }

    // ─────────────────────────────────────────────
    // Noise fracional (Fractal Brownian Motion)
    // ─────────────────────────────────────────────

    // fractalNoise2D: retorna valor em [0, 1]
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

    // fractalNoise3D: retorna valor em [0, 1]
    // scale: tamanho do bioma 3D (quanto maior, mais suave)
    // octaves: detalhamento (4~6 é bom para 3D também)
    float fractalNoise3D(float x, float y, float z, float scale = 80.0f, int octaves = 5) const {
        float value     = 0.0f;
        float amplitude = 1.0f;
        float frequency = 1.0f / scale;
        float maxVal    = 0.0f;

        for (int i = 0; i < octaves; i++) {
            value   += sample3D(x * frequency, y * frequency, z * frequency) * amplitude;
            maxVal  += amplitude;
            amplitude  *= 0.5f;
            frequency  *= 2.0f;
        }

        return value / maxVal; // normalizado [0, 1]
    }
};