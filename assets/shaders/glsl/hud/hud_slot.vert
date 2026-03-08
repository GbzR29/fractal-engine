#version 460 core

layout(location = 0) in vec2 aPos;

uniform vec3  color;
uniform float alpha;

out vec4 vColor;

void main() {
    gl_Position = vec4(aPos, 0.0, 1.0);
    vColor = vec4(color, alpha);
}
