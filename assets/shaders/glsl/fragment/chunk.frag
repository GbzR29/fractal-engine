#version 460 core

in vec2      TexCoord;
flat in int  TexLayer;
in float     LightFactor;

out vec4 FragColor;

uniform sampler2DArray texArray;

// ── Uniforms de iluminação — atualizados todo frame pelo SkySystem ─────────
uniform float sunIntensity;  // [0,1]  0=meia-noite, 1=meio-dia
uniform vec3  sunColor;      // cor da luz solar (branco quente→laranja→ausente)
uniform float ambientMin;    // luz mínima mesmo à noite (ex: 0.08)

void main() {
    vec4 texColor = texture(texArray, vec3(TexCoord, float(TexLayer)));

    if (texColor.a < 0.1)
        discard;

    // ── Cálculo de luz ────────────────────────────────────────────────────
    //
    // light = ambientMin                    (luz base — nunca zero)
    //       + LightFactor * sunIntensity    (contribuição direcional do sol)
    //
    // Clampado em [ambientMin, 1.0] para não estourar nem desaparecer.
    //
    // Ao multiplicar por sunColor:
    //   dia     → luz branca-amarelada normal
    //   pôr-sol → tinge tudo de laranja
    //   noite   → sunIntensity≈0, só ambientMin sobra (azul muito escuro)

    float light = clamp(ambientMin + LightFactor * sunIntensity, ambientMin, 1.0);
    vec3  lit   = texColor.rgb * sunColor * light;

    FragColor = vec4(lit, texColor.a);
}
