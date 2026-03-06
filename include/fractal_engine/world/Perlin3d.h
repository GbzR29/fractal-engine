#pragma once
#include <cmath>

// ─────────────────────────────────────────────────────────────────────────────
// Perlin3D — Noise determinístico 2D e 3D com Fractal Brownian Motion
// Substitui o antigo SimpleNoise.
// ─────────────────────────────────────────────────────────────────────────────
class Perlin3D {
public:
    unsigned int seed;

    explicit Perlin3D(unsigned int s = 1337) : seed(s) {}

    // ─────────────────────────────────────────────
    // Hash determinístico
    // ─────────────────────────────────────────────
    float hash2D(int x, int z) const {
        unsigned int n = (unsigned int)(x * 1619 + z * 31337 + seed * 1013904223);
        n = (n ^ (n >> 13)) * 1274126177u;
        n = (n ^ (n >> 16));
        return (float)(n & 0x7fffffff) / (float)0x7fffffff;
    }

    float hash3D(int x, int y, int z) const {
        unsigned int n = (unsigned int)(x * 1619 + y * 571 + z * 31337 + seed * 1013904223);
        n = (n ^ (n >> 13)) * 1274126177u;
        n = (n ^ (n >> 16));
        return (float)(n & 0x7fffffff) / (float)0x7fffffff;
    }

    // ─────────────────────────────────────────────
    // Interpolação
    // ─────────────────────────────────────────────
    static float lerp(float a, float b, float t) {
        return a + t * (b - a);
    }

    // Smoothstep cúbico — suaviza bordas entre células
    static float smooth(float t) {
        return t * t * (3.0f - 2.0f * t);
    }

    // ─────────────────────────────────────────────
    // Amostragem bilinear 2D
    // ─────────────────────────────────────────────
    float sample2D(float x, float z) const {
        int x0 = (int)std::floor(x), x1 = x0 + 1;
        int z0 = (int)std::floor(z), z1 = z0 + 1;
        float tx = smooth(x - x0);
        float tz = smooth(z - z0);
        return lerp(
            lerp(hash2D(x0,z0), hash2D(x1,z0), tx),
            lerp(hash2D(x0,z1), hash2D(x1,z1), tx),
            tz
        );
    }

    // ─────────────────────────────────────────────
    // Amostragem trilinear 3D
    // ─────────────────────────────────────────────
    float sample3D(float x, float y, float z) const {
        int x0 = (int)std::floor(x), x1 = x0 + 1;
        int y0 = (int)std::floor(y), y1 = y0 + 1;
        int z0 = (int)std::floor(z), z1 = z0 + 1;
        float tx = smooth(x - x0);
        float ty = smooth(y - y0);
        float tz = smooth(z - z0);

        float v00 = lerp(hash3D(x0,y0,z0), hash3D(x1,y0,z0), tx);
        float v10 = lerp(hash3D(x0,y1,z0), hash3D(x1,y1,z0), tx);
        float v01 = lerp(hash3D(x0,y0,z1), hash3D(x1,y0,z1), tx);
        float v11 = lerp(hash3D(x0,y1,z1), hash3D(x1,y1,z1), tx);

        return lerp(lerp(v00, v10, ty), lerp(v01, v11, ty), tz);
    }

    // ─────────────────────────────────────────────
    // Fractal Brownian Motion 2D — retorna [0, 1]
    // ─────────────────────────────────────────────
    float fractalNoise(float x, float z, float scale = 80.0f, int octaves = 5) const {
        float value = 0, amplitude = 1, frequency = 1.0f / scale, maxVal = 0;
        for (int i = 0; i < octaves; i++) {
            value    += sample2D(x * frequency, z * frequency) * amplitude;
            maxVal   += amplitude;
            amplitude  *= 0.5f;
            frequency  *= 2.0f;
        }
        return value / maxVal;
    }

    // ─────────────────────────────────────────────
    // Fractal Brownian Motion 3D — retorna [0, 1]
    // ─────────────────────────────────────────────
    float fractalNoise3D(float x, float y, float z, float scale = 80.0f, int octaves = 5) const {
        float value = 0, amplitude = 1, frequency = 1.0f / scale, maxVal = 0;
        for (int i = 0; i < octaves; i++) {
            value    += sample3D(x * frequency, y * frequency, z * frequency) * amplitude;
            maxVal   += amplitude;
            amplitude  *= 0.5f;
            frequency  *= 2.0f;
        }
        return value / maxVal;
    }
};