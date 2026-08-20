#version 440

QC_INCLUDE "customfrag.glsl"

void main()
{
    float a = 0.6 + 0.2 * sin(0.1 * fragCoord.x + 4.0 * iTime);
    vec4 color = vec4(a, a, a, 1.0);
    fragColor = sdfFontAlpha() * globalAlpha * color;
    // Note: Disable this if you don't want color effects to affect.
    applyColorEffects(fragColor);
}
