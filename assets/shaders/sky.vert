#version 460 core

out vec2 vNDC;

void main()
{
    // Fullscreen triangle — no VBO needed
    const vec2 pos[3] = vec2[3](
        vec2(-1.0, -1.0),
        vec2( 3.0, -1.0),
        vec2(-1.0,  3.0)
    );
    vNDC = pos[gl_VertexID];
    gl_Position = vec4(vNDC, 0.9999, 1.0);
}
