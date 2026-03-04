#version 460 core

out vec4 FragColor;

in vec2 TexCoord;
flat in int TexID;

uniform sampler2D texTop;
uniform sampler2D texBottom;
uniform sampler2D texSide;

void main() {
    vec4 texColor;

    if (TexID == 0)
        texColor = texture(texTop, TexCoord);
    else if (TexID == 1)
        texColor = texture(texBottom, TexCoord);
    else
        texColor = texture(texSide, TexCoord);

    FragColor = texColor;
}