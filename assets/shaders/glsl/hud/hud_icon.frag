#version 460 core

in  vec2 vUV;
out vec4 FragColor;

uniform sampler2DArray texArray;
uniform int            texLayer;

void main() {
    vec4 col = texture(texArray, vec3(vUV, float(texLayer)));
    if (col.a < 0.1) discard;

    // Leve escurecimento nas bordas para dar profundidade ao slot
    vec2  centered = vUV * 2.0 - 1.0;  // [-1, 1]
    float vignette = 1.0 - dot(centered, centered) * 0.15;
    FragColor = vec4(col.rgb * vignette, col.a);
}
