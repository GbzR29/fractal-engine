#version 460 core

in vec3 vFragPos;
in vec2 vTexCoords;
in mat3 vTBN;

out vec4 FragColor;

// ── PBR textures (slots definidos em PBRSlot::) ───────────────
uniform sampler2D uAlbedoMap;    // slot 0
uniform sampler2D uNormalMap;    // slot 1
uniform sampler2D uMetallicMap;  // slot 2
uniform sampler2D uRoughnessMap; // slot 3
uniform sampler2D uAOMap;        // slot 4
uniform sampler2D uEmissiveMap;  // slot 5

// ── PBR factors ───────────────────────────────────────────────
uniform vec4  uAlbedoFactor;
uniform float uMetallicFactor;
uniform float uRoughnessFactor;
uniform vec3  uEmissiveFactor;
uniform float uAlphaCutoff;

// ── Camera & lighting ─────────────────────────────────────────
uniform vec3 uCamPos;
uniform vec3 uLightDir;    // direção normalizada do sol (world space)
uniform vec3 uLightColor;  // irradiância da luz

const float PI = 3.14159265359;

// ── Cook-Torrance BRDF helpers ────────────────────────────────
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a   = roughness * roughness;
    float a2  = a * a;
    float NdH = max(dot(N, H), 0.0);
    float d   = NdH * NdH * (a2 - 1.0) + 1.0;
    return a2 / (PI * d * d);
}

float GeometrySchlick(float NdV, float roughness)
{
    float k = (roughness + 1.0);
    k = k * k / 8.0;
    return NdV / (NdV * (1.0 - k) + k);
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    return GeometrySchlick(max(dot(N, V), 0.0), roughness)
         * GeometrySchlick(max(dot(N, L), 0.0), roughness);
}

vec3 FresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

void main()
{
    // ── Amostra textures ──────────────────────────────────────
    vec4  albedoSamp = texture(uAlbedoMap, vTexCoords) * uAlbedoFactor;
    if (albedoSamp.a < uAlphaCutoff) discard;

    // Albedo em espaço linear
    vec3  albedo    = pow(albedoSamp.rgb, vec3(2.2));
    // glTF: metallic no canal .b, roughness no canal .g
    float metallic  = texture(uMetallicMap,  vTexCoords).b * uMetallicFactor;
    float roughness = texture(uRoughnessMap, vTexCoords).g * uRoughnessFactor;
    float ao        = texture(uAOMap,        vTexCoords).r;
    vec3  emissive  = texture(uEmissiveMap,  vTexCoords).rgb * uEmissiveFactor;

    roughness = clamp(roughness, 0.04, 1.0);

    // ── Normal (tangent-space → world-space via TBN) ──────────
    vec3 N = normalize(vTBN * (texture(uNormalMap, vTexCoords).rgb * 2.0 - 1.0));
    vec3 V = normalize(uCamPos - vFragPos);
    vec3 L = normalize(uLightDir);
    vec3 H = normalize(V + L);

    // ── Reflectância F0 ───────────────────────────────────────
    vec3 F0 = mix(vec3(0.04), albedo, metallic);

    // ── BRDF ─────────────────────────────────────────────────
    float NDF = DistributionGGX(N, H, roughness);
    float G   = GeometrySmith(N, V, L, roughness);
    vec3  F   = FresnelSchlick(max(dot(H, V), 0.0), F0);

    vec3  spec  = (NDF * G * F)
                / (4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001);
    vec3  kD    = (vec3(1.0) - F) * (1.0 - metallic);
    float NdL   = max(dot(N, L), 0.0);
    vec3  Lo    = (kD * albedo / PI + spec) * uLightColor * NdL;

    // ── Ambient + emissive ────────────────────────────────────
    vec3 color = vec3(0.03) * albedo * ao + Lo + emissive;

    // ── Reinhard tonemap + gamma correction ───────────────────
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0 / 2.2));

    FragColor = vec4(color, albedoSamp.a);
}
