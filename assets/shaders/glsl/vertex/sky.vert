#version 330 core

layout(location = 0) in vec3 aPos;

out vec3 vDir; // direção do raio a partir do centro do skybox

uniform mat4 view;
uniform mat4 projection;

void main() {
    vDir = aPos; // posição do cubo unitário = direção do raio
    // Remove translação da view matrix (skybox não se move com a câmera)
    mat4 viewNoTranslation = mat4(mat3(view));
    vec4 pos = projection * viewNoTranslation * vec4(aPos, 1.0);
    // Trick para colocar o skybox sempre no fundo (z/w = 1.0)
    gl_Position = pos.xyww;
}
