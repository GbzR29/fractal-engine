#version 330 core

in  vec3 vDir;
out vec4 FragColor;

// ── Uniforms do SkySystem ────────────────────────────────────────────────────
uniform float uTimeOfDay;    // [0,1]  0=meia-noite, 0.5=meio-dia
uniform float uSunIntensity; // [0,1]
uniform vec3  uSunColor;     // cor da luz solar

// ── Constantes ───────────────────────────────────────────────────────────────
const float PI = 3.14159265;

// ── Utilidades ───────────────────────────────────────────────────────────────
float smoothstep01(float edge0, float edge1, float x) {
    float t = clamp((x - edge0) / (edge1 - edge0), 0.0, 1.0);
    return t * t * (3.0 - 2.0 * t);
}

// Hash simples para pseudo-aleatoriedade
float hash(vec2 p) {
    p = fract(p * vec2(127.1, 311.7));
    p += dot(p, p + 19.19);
    return fract(p.x * p.y);
}

// ── Direção do Sol ───────────────────────────────────────────────────────────
// timeOfDay: 0.25=amanhecer (leste), 0.5=meio-dia (cima), 0.75=pôr-do-sol (oeste)
vec3 sunDirection() {
    float angle = (uTimeOfDay - 0.25) * 2.0 * PI; // 0 = leste, PI/2 = cima
    // Sol viaja no plano XY (leste→oeste), eixo Z=norte fixo
    return normalize(vec3(cos(angle), sin(angle), 0.2));
}

// ── Direção da Lua (oposta ao sol no ciclo) ──────────────────────────────────
vec3 moonDirection() {
    float angle = (uTimeOfDay + 0.25) * 2.0 * PI; // oposto ao sol
    return normalize(vec3(cos(angle), sin(angle), -0.1));
}

// ── Gradiente do céu ─────────────────────────────────────────────────────────
vec3 skyGradient(vec3 dir) {
    float y = clamp(dir.y, -1.0, 1.0); // altura: -1=baixo, 1=cima

    // Cores base do dia
    vec3 zenithDay    = vec3(0.10, 0.40, 0.80); // azul profundo no alto
    vec3 horizonDay   = vec3(0.55, 0.80, 0.95); // azul claro no horizonte
    // Cores do pôr/nascer do sol
    vec3 zenithDusk   = vec3(0.05, 0.05, 0.25); // roxo escuro no alto
    vec3 horizonDusk  = vec3(0.90, 0.40, 0.10); // laranja no horizonte
    // Cores da noite
    vec3 zenithNight  = vec3(0.01, 0.01, 0.05); // quase preto
    vec3 horizonNight = vec3(0.02, 0.03, 0.08); // azul noturno

    // Mistura vertical (horizonte → zênite)
    float hBlend = smoothstep01(-0.1, 0.6, y);

    vec3 colorDay  = mix(horizonDay,  zenithDay,  hBlend);
    vec3 colorDusk = mix(horizonDusk, zenithDusk, hBlend);
    vec3 colorNight= mix(horizonNight,zenithNight,hBlend);

    // Mistura dia ↔ dusk ↔ noite baseada em sunIntensity
    vec3 skyColor;
    if (uSunIntensity > 0.3) {
        float t = smoothstep01(0.3, 0.8, uSunIntensity);
        skyColor = mix(colorDusk, colorDay, t);
    } else {
        float t = smoothstep01(0.0, 0.3, uSunIntensity);
        skyColor = mix(colorNight, colorDusk, t);
    }

    return skyColor;
}

// ── Glow do horizonte ao amanhecer/entardecer ────────────────────────────────
vec3 horizonGlow(vec3 dir, vec3 sunDir) {
    // Intensidade do glow: máximo perto do horizonte e direção do sol
    float sunAngle  = dot(normalize(dir), normalize(sunDir));
    float horizonY  = 1.0 - abs(dir.y); // forte no horizonte
    float glowMask  = smoothstep01(0.5, 1.0, horizonY)
                    * smoothstep01(0.0, 0.8, sunAngle);
    // Glow só aparece quando sol está perto do horizonte
    float duskFactor = 1.0 - smoothstep01(0.2, 0.6, uSunIntensity);
    duskFactor      *= smoothstep01(0.0, 0.15, uSunIntensity);

    vec3 glowColor = mix(vec3(1.0, 0.3, 0.05), vec3(1.0, 0.65, 0.1),
                         smoothstep01(0.0, 0.5, uSunIntensity));
    return glowColor * glowMask * duskFactor * 1.2;
}

// ── Disco do Sol ─────────────────────────────────────────────────────────────
vec3 drawSun(vec3 dir, vec3 sunDir) {
    if (uSunIntensity < 0.01) return vec3(0.0);

    float cosA   = dot(normalize(dir), sunDir);
    float dist   = acos(clamp(cosA, -1.0, 1.0)); // ângulo em radianos

    float diskR  = 0.035; // raio do disco (~2°)
    float haloR  = 0.20;  // raio do halo suave

    // Disco central
    float disk   = 1.0 - smoothstep01(diskR * 0.8, diskR, dist);
    // Halo difuso ao redor
    float halo   = smoothstep01(haloR, diskR, dist) * 0.3;

    vec3  sunCore = mix(vec3(1.0, 0.9, 0.6), vec3(1.0, 1.0, 0.95),
                        smoothstep01(0.0, 0.5, uSunIntensity));

    return sunCore * (disk + halo) * uSunIntensity;
}

// ── Disco da Lua ─────────────────────────────────────────────────────────────
vec3 drawMoon(vec3 dir, vec3 moonDir) {
    float nightFactor = 1.0 - smoothstep01(0.0, 0.25, uSunIntensity);
    if (nightFactor < 0.01) return vec3(0.0);

    float cosA  = dot(normalize(dir), moonDir);
    float dist  = acos(clamp(cosA, -1.0, 1.0));

    float diskR = 0.025;
    float disk  = 1.0 - smoothstep01(diskR * 0.85, diskR, dist);
    // Halo suave da lua
    float halo  = smoothstep01(0.12, diskR, dist) * 0.15;

    vec3 moonColor = vec3(0.85, 0.88, 0.95); // branco azulado

    return moonColor * (disk + halo) * nightFactor;
}

// ── Campo de estrelas ─────────────────────────────────────────────────────────
// Projeção esférica → grade de células, uma estrela por célula
vec3 drawStars(vec3 dir) {
    float nightFactor = 1.0 - smoothstep01(0.05, 0.35, uSunIntensity);
    if (nightFactor < 0.01) return vec3(0.0);
    // Só acima do horizonte
    if (dir.y < -0.05) return vec3(0.0);

    // Converte direção para coordenadas esféricas
    float theta = atan(dir.z, dir.x);               // [−π, π]
    float phi   = asin(clamp(dir.y, -1.0, 1.0));    // [−π/2, π/2]

    // Grid com células de tamanho variável por camada (2 camadas = mais densidade)
    vec3 stars = vec3(0.0);

    // Camada 1 — estrelas grandes e brilhantes
    float scale1 = 80.0;
    vec2  cell1  = floor(vec2(theta, phi) * scale1);
    vec2  uv1    = fract(vec2(theta, phi) * scale1) - 0.5;
    float h1     = hash(cell1);
    float size1  = 0.08 + h1 * 0.06;
    float bright1= smoothstep01(0.85, 1.0, h1);         // só ~15% viram estrelas
    float d1     = length(uv1);
    stars += vec3(1.0, 0.95, 0.85)
           * bright1
           * (1.0 - smoothstep01(0.0, size1, d1))
           * 1.2;

    // Camada 2 — estrelas menores e mais numerosas
    float scale2 = 200.0;
    vec2  cell2  = floor(vec2(theta, phi) * scale2);
    vec2  uv2    = fract(vec2(theta, phi) * scale2) - 0.5;
    float h2     = hash(cell2 + vec2(53.7, 91.3));
    float size2  = 0.04 + h2 * 0.03;
    float bright2= smoothstep01(0.90, 1.0, h2);
    float d2     = length(uv2);
    stars += vec3(0.85, 0.90, 1.0)
           * bright2
           * (1.0 - smoothstep01(0.0, size2, d2))
           * 0.7;

    return stars * nightFactor;
}

// ── Main ─────────────────────────────────────────────────────────────────────
void main() {
    vec3 dir     = normalize(vDir);
    vec3 sunDir  = sunDirection();
    vec3 moonDir = moonDirection();

    // Monta o céu camada por camada
    vec3 color = skyGradient(dir);
    color += horizonGlow(dir, sunDir);
    color += drawStars(dir);
    color += drawMoon(dir, moonDir);
    color += drawSun(dir, sunDir);

    // Névoa suave embaixo do horizonte (terra/vazio)
    float belowHorizon = smoothstep01(0.0, -0.15, dir.y);
    vec3  groundFog    = mix(vec3(0.15, 0.12, 0.08), vec3(0.01, 0.01, 0.03),
                             smoothstep01(0.0, 1.0, uSunIntensity));
    color = mix(color, groundFog, belowHorizon);

    FragColor = vec4(color, 1.0);
}
