#version 460 core
layout (location = 0) in vec3 aPos;

out vec4 vertexColor;

// Uniforms para tornar a UI responsiva ao tamanho da janela
uniform float aspectRatio;

void main()
{
    // NDC com compensação de aspect ratio para manter proporções
    // Dividir X pelo aspect ratio mantém a crosshair circular em qualquer resolução
    vec3 adjustedPos = aPos;
    adjustedPos.x /= aspectRatio;
    
    gl_Position = vec4(adjustedPos, 1.0);
    vertexColor = vec4(1.0, 1.0, 1.0, 1.0);
}