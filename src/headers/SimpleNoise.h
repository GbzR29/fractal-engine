class SimpleNoise {
public:
    unsigned int seed;

    SimpleNoise(unsigned int s = 1337) : seed(s) {}

    float random(int x) const {
        x = (x << 13) ^ x;
        return 1.0f - ((x * (x * x * 15731 + 789221) + seed) & 0x7fffffff) / 1073741824.0f;
    }

    float random2D(int x, int z) const {
        int n = x * 374761393 + z * 668265263 + seed * 31;
        n = (n ^ (n >> 13)) * 1274126177;
        return (n & 0x7fffffff) / 2147483648.0f;
    }


    float lerp(float a, float b, float t) const {
        return a + t * (b - a);
    }

    float smooth(float t) const {
        return t * t * (3 - 2 * t); // smoothstep
    }

    float sample(float x) const {
        int x0 = (int)floor(x);
        int x1 = x0 + 1;

        float t = smooth(x - x0);

        return lerp(random(x0), random(x1), t);
    }

    float sample2D(float x, float z) const {
        int x0 = (int)floor(x);
        int x1 = x0 + 1;
        int z0 = (int)floor(z);
        int z1 = z0 + 1;

        float tx = smooth(x - x0);
        float tz = smooth(z - z0);

        float v00 = random2D(x0, z0);
        float v10 = random2D(x1, z0);
        float v01 = random2D(x0, z1);
        float v11 = random2D(x1, z1);

        float a = lerp(v00, v10, tx);
        float b = lerp(v01, v11, tx);

        return lerp(a, b, tz);
    }

    float fractalNoise(float x, float z) const {
        float value = 0.0f;
        float amplitude = 1.0f;
        float frequency = 0.01f;
        float max = 0.0f;

        for (int i = 0; i < 4; i++) {
            value += sample2D(x * frequency, z * frequency) * amplitude;
            max += amplitude;

            amplitude *= 0.5f;
            frequency *= 2.0f;
        }

        return value / max;
    }


};
