#version 330 core

in  vec3 vDir;
out vec4 FragColor;

uniform float uTimeOfDay;
uniform float uSunIntensity;
uniform vec3  uSunColor;

const float PI = 3.14159265;

float smoothstep01(float edge0, float edge1, float x) {
    float t = clamp((x - edge0) / (edge1 - edge0), 0.0, 1.0);
    return t * t * (3.0 - 2.0 * t);
}

// ── Hash functions ────────────────────────────────────────────────────────────
// Baseadas em operações bit a bit simuladas em float — muito estáveis no MinGW
float hash21(vec2 p) {
    p = fract(p * vec2(127.1, 311.7));
    p += dot(p, p + 19.19);
    return fract(p.x * p.y);
}

float hash21b(vec2 p) {
    p = fract(p * vec2(269.5, 183.3));
    p += dot(p, p + 47.53);
    return fract(p.x * p.y);
}

// ── Direções ─────────────────────────────────────────────────────────────────
vec3 sunDirection() {
    float angle = (uTimeOfDay - 0.25) * 2.0 * PI;
    return normalize(vec3(cos(angle), sin(angle), 0.2));
}
vec3 moonDirection() {
    float angle = (uTimeOfDay + 0.25) * 2.0 * PI;
    return normalize(vec3(cos(angle), sin(angle), -0.1));
}

// ── Gradiente do céu ─────────────────────────────────────────────────────────
vec3 skyGradient(vec3 dir) {
    float y = clamp(dir.y, -1.0, 1.0);

    vec3 zenithDay    = vec3(0.10, 0.40, 0.80);
    vec3 horizonDay   = vec3(0.55, 0.80, 0.95);
    vec3 zenithDusk   = vec3(0.05, 0.05, 0.25);
    vec3 horizonDusk  = vec3(0.90, 0.40, 0.10);
    vec3 zenithNight  = vec3(0.01, 0.01, 0.05);
    vec3 horizonNight = vec3(0.02, 0.03, 0.08);

    float hBlend    = smoothstep01(-0.1, 0.6, y);
    vec3 colorDay   = mix(horizonDay,   zenithDay,   hBlend);
    vec3 colorDusk  = mix(horizonDusk,  zenithDusk,  hBlend);
    vec3 colorNight = mix(horizonNight, zenithNight, hBlend);

    if (uSunIntensity > 0.3) {
        return mix(colorDusk, colorDay, smoothstep01(0.3, 0.8, uSunIntensity));
    } else {
        return mix(colorNight, colorDusk, smoothstep01(0.0, 0.3, uSunIntensity));
    }
}

// ── Glow do horizonte ─────────────────────────────────────────────────────────
vec3 horizonGlow(vec3 dir, vec3 sunDir) {
    float sunAngle   = dot(normalize(dir), normalize(sunDir));
    float horizonY   = 1.0 - abs(dir.y);
    float glowMask   = smoothstep01(0.5, 1.0, horizonY) * smoothstep01(0.0, 0.8, sunAngle);
    float duskFactor = (1.0 - smoothstep01(0.2, 0.6, uSunIntensity))
                     * smoothstep01(0.0, 0.15, uSunIntensity);
    vec3 glowColor   = mix(vec3(1.0, 0.3, 0.05), vec3(1.0, 0.65, 0.1),
                           smoothstep01(0.0, 0.5, uSunIntensity));
    return glowColor * glowMask * duskFactor * 1.2;
}

// ── Disco do Sol ─────────────────────────────────────────────────────────────
vec3 drawSun(vec3 dir, vec3 sunDir) {
    if (uSunIntensity < 0.01) return vec3(0.0);
    float dist = acos(clamp(dot(normalize(dir), sunDir), -1.0, 1.0));
    float disk = 1.0 - smoothstep01(0.028, 0.035, dist);
    float halo = smoothstep01(0.20, 0.035, dist) * 0.3;
    vec3  core = mix(vec3(1.0, 0.9, 0.6), vec3(1.0, 1.0, 0.95),
                     smoothstep01(0.0, 0.5, uSunIntensity));
    return core * (disk + halo) * uSunIntensity;
}

// ── Disco da Lua ──────────────────────────────────────────────────────────────
vec3 drawMoon(vec3 dir, vec3 moonDir) {
    float night = 1.0 - smoothstep01(0.0, 0.25, uSunIntensity);
    if (night < 0.01) return vec3(0.0);
    float dist = acos(clamp(dot(normalize(dir), moonDir), -1.0, 1.0));
    float disk = 1.0 - smoothstep01(0.021, 0.025, dist);
    float halo = smoothstep01(0.12, 0.025, dist) * 0.15;
    return vec3(0.85, 0.88, 0.95) * (disk + halo) * night;
}

// ── Campo de estrelas ─────────────────────────────────────────────────────────
//
// Técnica: projeção cúbica em 3 faces ortogonais separadas.
// Cada face usa uma grade 2D independente, eliminando completamente a distorção
// de polo que ocorre na projeção esférica (theta/phi).
//
// O céu é dividido em 3 pares de faces do cubo:
//   Face XY  — olhando para +Z e -Z
//   Face XZ  — olhando para +Y e -Y (teto e chão)
//   Face YZ  — olhando para +X e -X
//
// Cada face usa coordenadas locais 2D normalizadas, sem distorção.
// A transição entre faces é suavizada pelo peso (o ângulo de incidência).
// ─────────────────────────────────────────────────────────────────────────────
vec3 starLayer(vec2 uv, float scale, float threshold, float seed) {
    vec2  sc     = uv * scale;
    vec2  cellID = floor(sc);
    vec2  cellUV = fract(sc) - 0.5; // [-0.5, 0.5]

    // Centro aleatório dentro da célula
    vec2  offset = vec2(hash21(cellID + seed), hash21b(cellID + seed)) - 0.5;
    vec2  delta  = cellUV - offset * 0.7; // offset em 70% da célula
    float dist   = length(delta);

    float h = hash21(cellID + seed + 7.3);
    if (h < threshold) return vec3(0.0);

    // Tamanho angular da estrela
    float sz   = 0.04 + hash21b(cellID + seed) * 0.06;
    float glow = exp(-dist * dist / (sz * sz));

    // Cor variável
    float ch = hash21(cellID * 1.7 + seed);
    vec3  col = mix(
        mix(vec3(1.0, 0.93, 0.75), vec3(0.75, 0.87, 1.0), ch),
        vec3(1.0, 1.0, 0.97),
        smoothstep01(0.85, 1.0, h)
    );

    return col * glow * smoothstep01(threshold, 1.0, h);
}

vec3 drawStars(vec3 dir) {
    float night = 1.0 - smoothstep01(0.05, 0.35, uSunIntensity);
    if (night < 0.01) return vec3(0.0);

    vec3 d   = normalize(dir);
    vec3 col = vec3(0.0);

    // Pesos de cada face (produto escalar com a normal da face)
    // Evita estrelas "fantasmas" nas bordas usando suavização cúbica
    float wx = abs(d.x);
    float wy = abs(d.y);
    float wz = abs(d.z);
    // Eleva ao cubo para transição mais suave
    wx = wx * wx * wx;
    wy = wy * wy * wy;
    wz = wz * wz * wz;
    float wTotal = wx + wy + wz + 0.0001;

    // ── Face Z (front/back) ───────────────────────────────────────────────
    if (wz > 0.001) {
        vec2 uv   = d.xy / (abs(d.z) + 0.001);
        float w   = wz / wTotal;
        col += starLayer(uv, 12.0, 0.78, 0.0)  * w;
        col += starLayer(uv, 25.0, 0.84, 13.5) * w * 0.6;
    }

    // ── Face Y (top/bottom) ───────────────────────────────────────────────
    if (wy > 0.001) {
        vec2 uv   = d.xz / (abs(d.y) + 0.001);
        float w   = wy / wTotal;
        col += starLayer(uv, 12.0, 0.78, 31.7) * w;
        col += starLayer(uv, 25.0, 0.84, 47.2) * w * 0.6;
    }

    // ── Face X (left/right) ───────────────────────────────────────────────
    if (wx > 0.001) {
        vec2 uv   = d.yz / (abs(d.x) + 0.001);
        float w   = wx / wTotal;
        col += starLayer(uv, 12.0, 0.78, 61.4) * w;
        col += starLayer(uv, 25.0, 0.84, 79.8) * w * 0.6;
    }

    // Fade no horizonte
    float fade = smoothstep01(0.0, 0.07, d.y);
    return col * night * fade;
}

// ── Main ─────────────────────────────────────────────────────────────────────
void main() {
    vec3 dir     = normalize(vDir);
    vec3 sunDir  = sunDirection();
    vec3 moonDir = moonDirection();

    vec3 color = skyGradient(dir);
    color += horizonGlow(dir, sunDir);
    color += drawStars(dir);
    color += drawMoon(dir, moonDir);
    color += drawSun(dir, sunDir);

    // Névoa abaixo do horizonte
    float below = smoothstep01(0.0, -0.15, dir.y);
    vec3  fog   = mix(vec3(0.15, 0.12, 0.08), vec3(0.01, 0.01, 0.03),
                      smoothstep01(0.0, 1.0, uSunIntensity));
    color = mix(color, fog, below);

    FragColor = vec4(color, 1.0);
}
