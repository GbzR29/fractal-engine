#version 460 core

out vec4 FragColor;

in vec2 TexCoord;
flat in int TexID;

uniform sampler2D texTop;
uniform sampler2D texBottom;
uniform sampler2D texSide;
uniform sampler2D texStone;

void main() {
    vec4 texColor;

    if (TexID == 0)
        texColor = texture(texTop, TexCoord);      // Topo da grama
    else if (TexID == 1)
        texColor = texture(texBottom, TexCoord);   // Fundo (terra)
    else if (TexID == 2)
        texColor = texture(texSide, TexCoord);     // Lado da grama
    else if (TexID == 3)
        texColor = texture(texStone, TexCoord);    // Pedra
    else
        texColor = vec4(1.0, 0.0, 1.0, 1.0);      // Erro - rosa

    FragColor = texColor;
}