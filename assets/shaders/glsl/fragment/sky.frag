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
// Projeção cúbica: cada face do cubo usa grade 2D independente.
// Estrelas são PONTOS pequenos e nítidos — sem glow exagerado.
//
// Na projeção cúbica, as coordenadas UV podem variar bastante dependendo
// do ângulo de incidência, então usamos escala relativa ao tamanho da célula.
// ─────────────────────────────────────────────────────────────────────────────
vec3 starLayer(vec2 uv, float scale, float threshold, float seed) {
    vec2 sc     = uv * scale;
    vec2 cellID = floor(sc);
    vec2 cellUV = fract(sc); // [0, 1]

    // Posição da estrela dentro da célula (25%~75% para não ficar na borda)
    float ox = 0.25 + hash21(cellID + seed) * 0.5;
    float oy = 0.25 + hash21b(cellID + seed) * 0.5;

    float h = hash21(cellID + seed + 3.1);
    if (h < threshold) return vec3(0.0);

    // Distância ao centro da estrela em pixels de célula
    vec2  delta = cellUV - vec2(ox, oy);
    float dist  = length(delta);

    // Tamanho MUITO pequeno — estrelas são pontos, não bolas
    // 0.04 = ~4% do tamanho da célula = ponto nítido
    float sz    = 0.035 + hash21b(cellID + seed + 7.7) * 0.025;

    // Perfil sharp: cai rapidamente (exp com sigma pequeno)
    float glow  = exp(-dist * dist / (sz * sz * 0.5));

    // Sem multiplicador extra de brilho — mantém discreto
    float bright = smoothstep01(threshold, 1.0, h);

    // Cor: maioria branca/azulada, algumas levemente amareladas
    float ch = hash21(cellID * 2.3 + seed);
    vec3  col = mix(vec3(0.85, 0.92, 1.0), vec3(1.0, 0.97, 0.85), ch * ch);

    return col * glow * bright;
}

vec3 drawStars(vec3 dir) {
    float night = 1.0 - smoothstep01(0.05, 0.35, uSunIntensity);
    if (night < 0.01) return vec3(0.0);

    vec3 d   = normalize(dir);
    vec3 col = vec3(0.0);

    // Pesos de face — elevado ao cubo para transição suave e invisível
    float wx = abs(d.x); wx = wx * wx * wx;
    float wy = abs(d.y); wy = wy * wy * wy;
    float wz = abs(d.z); wz = wz * wz * wz;
    float wt = wx + wy + wz + 0.0001;

    // ── Face Z ────────────────────────────────────────────────────────────
    {
        vec2  uv = d.xy / (abs(d.z) + 0.001);
        float w  = wz / wt;
        // escala 18: ~324 células por face → densidade razoável de estrelas
        col += starLayer(uv, 18.0, 0.82, 0.00) * w;
        // escala 38: estrelas menores e mais numerosas no background
        col += starLayer(uv, 38.0, 0.88, 11.3) * w * 0.55;
    }

    // ── Face Y (teto) ─────────────────────────────────────────────────────
    {
        vec2  uv = d.xz / (abs(d.y) + 0.001);
        float w  = wy / wt;
        col += starLayer(uv, 18.0, 0.82, 29.7) * w;
        col += starLayer(uv, 38.0, 0.88, 43.1) * w * 0.55;
    }

    // ── Face X ────────────────────────────────────────────────────────────
    {
        vec2  uv = d.yz / (abs(d.x) + 0.001);
        float w  = wx / wt;
        col += starLayer(uv, 18.0, 0.82, 57.4) * w;
        col += starLayer(uv, 38.0, 0.88, 73.9) * w * 0.55;
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
