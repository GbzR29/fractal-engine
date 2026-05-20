#version 460 core

in  vec2 vNDC;
out vec4 fragColor;

uniform mat4 uInvViewProj;

void main()
{
    // Reconstruct world-space view direction
    vec4 worldPos = uInvViewProj * vec4(vNDC, 1.0, 1.0);
    vec3 dir = normalize(worldPos.xyz / worldPos.w);

    float y = dir.y;

    // Sky: zenith -> horizon
    vec3 zenith  = vec3(0.10, 0.24, 0.52);  // deep blue
    vec3 horizon = vec3(0.52, 0.72, 0.88);  // light blue / haze

    // Void: horizon -> deep dark
    vec3 voidCol = vec3(0.03, 0.03, 0.04);

    vec3 color;
    if (y >= 0.0)
    {
        float t = pow(y, 0.45);             // non-linear — wider horizon band
        color = mix(horizon, zenith, t);
    }
    else
    {
        float t = clamp(pow(-y * 2.5, 0.6), 0.0, 1.0);
        color = mix(horizon * 0.55, voidCol, t);
    }

    fragColor = vec4(color, 1.0);
}
