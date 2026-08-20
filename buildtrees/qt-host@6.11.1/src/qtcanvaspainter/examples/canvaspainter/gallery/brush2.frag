#version 440

QC_INCLUDE "customfrag.glsl"

void main()
{
    //vec4 plasmaColors = vec4(1.0, 0.0, 0.0, 1.0);
    vec4 plasmaColors = data1;
    // These could also be taken from data inputs
    float plasmaScale = 0.1;
    float time = iTime * 0.1;
    vec2 resolution = vec2(400, 400);
    vec2 a = vec2(resolution.x / resolution.y, 1.0);
    vec2 c = 0.01 * fragCoord.xy * a * 2.0 + time * 0.45;
    float k = 0.25 + cos(c.y + sin(.148 - time)) + 3.2 * time;
    float w = 0.9 + sin(c.x + cos(.628 + time)) - 1.2 * time;
    float d = length(c);
    float s = (2.0 / (plasmaScale + 0.02)) * cos(d+w) * sin(k+w);
    vec4 plasma = vec4(0.5 + 0.5 * cos(s + plasmaColors.rgb), 1.0);
    // Note: If you don't need clipping support, remove clipMask().
    // Note: If you don't need antialiasing support, remove antialiasingAlpha().
    //fragColor = plasma * globalAlpha * antialiasingAlpha() * clipMask();
    fragColor = plasma * globalAlpha * antialiasingAlpha();

    // Note: Disable this if you don't want color effects to affect.
    applyColorEffects(fragColor);
}
