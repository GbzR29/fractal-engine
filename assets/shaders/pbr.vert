#version 460 core

layout(location = 0) in vec3  aPosition;
layout(location = 1) in vec3  aNormal;
layout(location = 2) in vec2  aTexCoords;
layout(location = 3) in vec3  aTangent;
layout(location = 4) in vec3  aBitangent;
layout(location = 5) in ivec4 aBoneIDs;
layout(location = 6) in vec4  aBoneWeights;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;
uniform mat4 uBoneMatrices[100];
uniform bool uSkinned;

out vec3 vFragPos;
out vec2 vTexCoords;
out mat3 vTBN;

void main()
{
    vec4 localPos  = vec4(aPosition, 1.0);
    vec3 localNorm = aNormal;
    vec3 localTang = aTangent;

    if (uSkinned) {
        mat4 skin =
            uBoneMatrices[aBoneIDs[0]] * aBoneWeights[0] +
            uBoneMatrices[aBoneIDs[1]] * aBoneWeights[1] +
            uBoneMatrices[aBoneIDs[2]] * aBoneWeights[2] +
            uBoneMatrices[aBoneIDs[3]] * aBoneWeights[3];
        localPos  = skin * localPos;
        localNorm = mat3(skin) * localNorm;
        localTang = mat3(skin) * localTang;
    }

    vec4 worldPos = uModel * localPos;
    vFragPos      = worldPos.xyz;
    vTexCoords    = aTexCoords;

    mat3 normalMatrix = transpose(inverse(mat3(uModel)));
    vec3 N = normalize(normalMatrix * localNorm);
    vec3 T = normalMatrix * localTang;

    // Proteção contra tangente zero (modelos sem UVs ou sem tangentes geradas)
    if (dot(T, T) < 1e-6) {
        vec3 up = abs(N.y) < 0.99 ? vec3(0.0, 1.0, 0.0) : vec3(1.0, 0.0, 0.0);
        T = cross(up, N);
    }
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T);
    vTBN   = mat3(T, B, N);

    gl_Position = uProjection * uView * worldPos;
}
