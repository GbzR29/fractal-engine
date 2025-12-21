#version 460 core

out vec4 FragColor;

in vec2 TexCoord;
flat in int TexID;

uniform sampler2D texture1;
uniform sampler2D texture2;
uniform sampler2D texture3;

void main()
{
    if (TexID == 0) {
        FragColor = texture(texture1, TexCoord);
        //FragColor = vec4(0.0, 0.0, 1.0, 1.0);
    }
    else if (TexID == 1) {
        FragColor = texture(texture2, TexCoord);
        //FragColor = vec4(0.0, 1.0, 0.0, 1.0);
    }
    else if (TexID == 2) {
        FragColor = texture(texture3, TexCoord);
        //FragColor = vec4(1.0, 0.0, 0.0, 1.0);
    }    
}